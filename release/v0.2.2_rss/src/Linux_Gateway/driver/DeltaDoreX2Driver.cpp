#include "assert.h"
#include "driver/DeltaDoreX2Driver.h"
#include <unistd.h>

#include "rollershutter/RollerShutterCommandArg.h"
#include "light/LightColorArg.h"

#include "Utils.h"

using namespace deltadoreX2d;
using namespace std;

#define UPDATE_REFRESH_TIME(device) device->setNextRefreshTs(Helper::getCurrentSystemMS())
#define UPDATE_DETECT_TIME(device) device->setNextRefreshTs(Helper::getCurrentSystemMS()+30*1000)
#define RESET_REFRESH_TIME(device) device->setNextRefreshTs(Helper::getCurrentSystemMS()+60*1000); \
    logger(LEVEL_DEBUG, "Set next query time after %d Sec\n", 60)

#define SIMPLE_SUCCESS_RESPONSE(root)                   \
    cJSON_AddStringToObject(root,"message","success");  \
    cJSON_AddBoolToObject(root,"success", true)

#define SIMPLE_ERROR_RESPONSE(root, errMsg)             \
    cJSON_AddStringToObject(root,"message",errMsg);     \
    cJSON_AddBoolToObject(root,"success", false)

#define PREPARE_REQUEST(root, network, node)                        \
    char errMsg[100];                                               \
    Network* net = checkNetwork(network, errMsg, sizeof(errMsg));   \
    if (net == NULL){                                               \
        SIMPLE_ERROR_RESPONSE(root, errMsg);                        \
        return root;                                                \
    }                                                               \
    if (!checkNode(net, node, errMsg, sizeof(errMsg))){             \
        SIMPLE_ERROR_RESPONSE(root, errMsg);                        \
        return root;                                                \
    }


#define NAMETYPE_NETWORK "network"
#define NAMETYPE_NODE "node"
#define CONST_NO_NAME "NO_NAME"


#define LOCK_CMD_HANDLING sem_wait(&semHandling)
#define UNLOCK_CMD_HANDLING sem_post(&semHandling)

static const char * DEVICE_TYPE_NAME_LIGHT = "light";
static const char * DEVICE_TYPE_NAME_ROLLERSHUTTER = "rollerShutter";
static const char * DEVICE_TYPE_NAME_UNKNOWN = "unknown";

DeltaDoreX2Driver::DeltaDoreX2Driver() : controller(NULL), acked(false)
{
    //ctor
    for(int i=0;i<MAX_NETWORK_NUM;i++){
        for(int j=0;j<MAX_NODE_NUM;j++){
            allDeviceInfo[i][j] = NULL;
        }
    }
}

DeltaDoreX2Driver::~DeltaDoreX2Driver()
{
    //dtor
    if (controller != NULL){
        printf("Close controller\n");
        controller->close();
    }
    sem_destroy(&semAck);
    sem_destroy(&semHandling);

    for(int i=0;i<MAX_NETWORK_NUM;i++){
        for(int j=0;j<MAX_NODE_NUM;j++){
            if (allDeviceInfo[i][j] != NULL) {
                delete allDeviceInfo[i][j];
                allDeviceInfo[i][j] = NULL;
            }
        }
    }
}
void DeltaDoreX2Driver::init(const char* devName)
{
    controller = NULL;
    // self init
    sem_init(&semAck, 0, 0);
    sem_init(&semHandling, 0, 1);
    // init serial port device
    device.init(devName);
    if (!device.initSuccess()){
        printf("Device %s initial failed\n", devName);
        return;
    }
    Controller* ctrl = Factory::createController();
    ctrl->addAcknowledgmentListener(this);
    MeshController* meshctrl = ctrl->convert<MeshController*>();
    meshctrl->addEndTransactionListener(this);
    meshctrl->addNodeDiscoveredListener(this);

    ctrl->open(&device, &device);
    trans_end_flag = false;
    meshctrl->initNetworks();
    controller = ctrl;

    waitAck(60);

    int nn = meshctrl->getNetworkCount();
    for(int i = 0; i < nn; i++)
    {
        Network* pNetwork = meshctrl->getNetwork(i);
        if (pNetwork == NULL)
        {
            continue;
        }

        vector<Node> nodes = pNetwork->getTopology();
        vector<Node>::iterator it;
        for(it=nodes.begin(); it != nodes.end(); it++)
        {
            Node node = *it;
            int deviceType = tryToGetDeviceType(i, node.toInt());
            switch (deviceType){
            case DeltaDoreDeviceInfo::DEVICE_TYPE_LIGHT:
                printf("Node %d in network %d is %s\n", node.toInt(), i, DEVICE_TYPE_NAME_LIGHT);
                break;
            case DeltaDoreDeviceInfo::DEVICE_TYPE_ROLLER_SHUTTER:
                printf("Node %d in network %d is %s\n", node.toInt(), i, DEVICE_TYPE_NAME_ROLLERSHUTTER);
                break;
            default:
                printf("Node %d in network %d is %s\n", node.toInt(), i, DEVICE_TYPE_NAME_UNKNOWN);
            }
        }
    }
}


void DeltaDoreX2Driver::acknowledgment(const AcknowledgmentEvent& evt)
{
    const Acknowledgment ack = evt.getAcknowledgment();
    printf("got ACK %s\n", ack.toString().c_str());
    acked = (ack == Acknowledgment::ACK || ack==Acknowledgment::NACK);

    // at last, release ACK semaphore
    if (ack == Acknowledgment::ACK || ack==Acknowledgment::NACK)
    {
        //sem_post(&semAck);
        trans_end_flag = true;
    }
}
void DeltaDoreX2Driver::waitAck(int seconds)
{
    //sem_wait(&semAck);
    for(int i=0;i<seconds*5;i++){
        Helper::sleep_ms(200);
        if (trans_end_flag){
            break;
        }
    }
    Helper::sleep_ms(100);
}
Request* DeltaDoreX2Driver::createRequest(RequestClass reqClass)
{
    MeshController* mCtrl = controller->convert<MeshController*>();
    Request *req = mCtrl->createRequest(reqClass);
    setContextRequestClass(reqClass);
    return req;
}
void DeltaDoreX2Driver::beginTransaction(Request* req){
    MeshController* mCtrl = controller->convert<MeshController*>();
    trans_end_flag = false;
    mCtrl->beginTransaction(req);
}

void DeltaDoreX2Driver::endTransaction(const EndTransactionEvent& evt)
{
    logger(LEVEL_DEBUG, "End transaction\n");

    std::vector<Node> nodes = evt.getRequest()->nodes();
    std::vector<Node>::iterator it;
    for(it = nodes.begin(); it != nodes.end(); it++)
    {
        Node node = *it;
        Response* resp = evt.getResponse(node)->clone();
        logger(LEVEL_DEBUG, "Response status is %s\n", resp->getStatus().toString().c_str());
        if (resp->getStatus() == ResponseStatus::UNEXISTING_NODE || resp->getStatus() == ResponseStatus::UNREACHABLE_NODE) {
            DeltaDoreDeviceInfo* pDevice = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());
            // Never got offline. So never change DeviceType any more. For customer emergency change.
            //pDevice->setDeviceType(DeltaDoreDeviceInfo::DEVICE_TYPE_UNKNOWN);
        }
        if(resp->instanceOf (RollerShutterStatusResponse_t))
        {
            logger(LEVEL_DEBUG, "Response is RollerShutterStatusResponse_t\n");
            RollerShutterStatusResponse* shutterResp = resp->convert<RollerShutterStatusResponse*>();
            onRollerShutterStatusResponse(*shutterResp);
        }else if (resp->instanceOf(LightStatusResponse_t)){
            logger(LEVEL_DEBUG, "Response is LightStatusResponse_t\n");
            LightStatusResponse* lightResp = resp->convert<LightStatusResponse*>();
            onLightStatusResponse(*lightResp);
        }else if (resp->instanceOf(LightColorResponse_t)){
            logger(LEVEL_DEBUG, "Response is LightColorResponse_t\n");
            LightColorResponse* lightResp = resp->convert<LightColorResponse*>();
            onLightColorResponse(*lightResp);
        }else if (resp->instanceOf(LightInfoResponse_t)){
            logger(LEVEL_DEBUG, "Response is LightInfoResponse_t\n");
            LightInfoResponse* lightResp = resp->convert<LightInfoResponse*>();
            onLightInfoResponse(*lightResp);
        }else if (resp->instanceOf(RollerShutterInfoResponse_t)){
            logger(LEVEL_DEBUG, "Response is RollerShutterInfoResponse_t\n");
            RollerShutterInfoResponse* shutterResp = resp->convert<RollerShutterInfoResponse*>();
            onRollerShutterInfoResponse(*shutterResp);
        }else {
            logger(LEVEL_DEBUG, "Response is CoreResponse\n");
            DeltaDoreDeviceInfo* pDevice = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());
            pDevice->setLastResponseStatus(resp->getStatus());
            pDevice->setOnLine(false);
            //pDevice->setNextRefreshTs(Helper::getCurrentSystemMS()+30*1000);
            RESET_REFRESH_TIME(pDevice);
        }

    }

}

DeltaDoreDeviceInfo* DeltaDoreX2Driver::getDeviceInfo(int network, int node)
{
    if (network < 0 || network >= MAX_NETWORK_NUM || node < 0 || node >= MAX_NODE_NUM){
        logger(LEVEL_ERROR, "Query invalid device[%d],[%d]", network, node);
        return &idleDeviceInfo;
    }
    DeltaDoreDeviceInfo* pDev = allDeviceInfo[network][node];
    if (pDev == NULL)
    {
        pDev = new DeltaDoreDeviceInfo();
        pDev->setDeviceType(DeltaDoreDeviceInfo::DEVICE_TYPE_UNKNOWN); // initial as unknown
        allDeviceInfo[network][node] = pDev;
    }
    // TODO debug
    // printf("device info of (%d.%d) is %p\n", network, node, pDev);
    return pDev;
}

void DeltaDoreX2Driver::setContextRequestClass(RequestClass reqClass)
{
    contextRequestClass=reqClass;
    if (contextRequestClass == CurrentConsumptionRequest_t)
    {
        contextResponseClass = ThermicSystemStatusResponse_t;
    }
    else
    {
        contextResponseClass=CurrentConsumptionResponse_t;
    }
}

Network* DeltaDoreX2Driver::checkNetwork(int network, char* errMsg, int msgLen)
{
    if (network >= MAX_NETWORK_NUM){
        snprintf(errMsg, msgLen, "network ID max value is %d", MAX_NETWORK_NUM-1);
        return NULL;
    }

    MeshController* mCtrl = controller->convert<MeshController*>();
    Network* pNetwork = mCtrl->getNetwork(network);
    if (pNetwork == NULL)
    {
        snprintf(errMsg, msgLen, "network %d not exist", network);
        return NULL;
    }
    return pNetwork;
}

bool DeltaDoreX2Driver::checkNode(Network* pNetwork, int node, char* errMsg, int msgLen)
{
    assert(pNetwork != NULL);
    if (node >= MAX_NODE_NUM){
        snprintf(errMsg, msgLen, "node ID max value is %d", MAX_NODE_NUM-1);
        return false;
    }
    vector<Node> nodes = pNetwork->getTopology();
    vector<Node>::iterator it;
    for(it=nodes.begin(); it != nodes.end(); it ++){
        Node cNode = * it;
        if (cNode.toInt() == node){
            return true;
        }
    }
    snprintf(errMsg, msgLen, "node %d not existed in network %d", node, pNetwork->getIdentifier());
    return false;
}


void DeltaDoreX2Driver::onRollerShutterStatusResponse(RollerShutterStatusResponse& response)
{
    DeltaDoreDeviceInfo* device = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());

    setContextResponseClass(RollerShutterStatusResponse_t);
    device->setDeviceType(DeltaDoreDeviceInfo::DEVICE_TYPE_ROLLER_SHUTTER);
    device->setLastResponseStatus(response.getStatus());

    device->setPosition(response.getPosition());
    device->setFavoritePosition(response.isFavoritePosition());
    device->setIntrusionDetected(response.isIntrusionDetected());
    device->setLoweringFaulty(response.isLoweringFaulty());
    device->setObstacleFaulty(response.isObstacleFaulty());
    device->setOverheatingFaulty(response.isOverheatingFaulty());
    device->setRaisingFaulty(response.isRaisingFaulty());
    device->setOnLine(true);
    RESET_REFRESH_TIME(device);

}
void DeltaDoreX2Driver::onRollerShutterInfoResponse(RollerShutterInfoResponse& response)
{
    DeltaDoreDeviceInfo* device = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());
    setContextResponseClass(RollerShutterInfoResponse_t);
    device->setDeviceType(DeltaDoreDeviceInfo::DEVICE_TYPE_ROLLER_SHUTTER);
    device->setLastResponseStatus(response.getStatus());

    device->setChannelCount(response.getChannelCount());
    //printf("onRollerShutterInfoResponse actuatorType is %s(%d)\n", response.getActuatorType().toString().c_str(), response.getActuatorType().toInt());
    device->setOnLine(true);
    device->setRollerShutterActuatorType(response.getActuatorType());
}
void DeltaDoreX2Driver::onLightStatusResponse(LightStatusResponse& response)
{
    DeltaDoreDeviceInfo* device = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());
    setContextResponseClass(LightStatusResponse_t);
    device->setDeviceType(DeltaDoreDeviceInfo::DEVICE_TYPE_LIGHT);
    device->setOnLine(true);
    device->setLastResponseStatus(response.getStatus());

    device->setLevel(response.getLevel());
    device->setOverloadFaulty(response.isOverloadFaulty());
    device->setCommandFaulty(response.isCommandFaulty());
    device->setOverheatingFaulty(response.isOverheatingFaulty());
    device->setFavoritePosition(response.isFavoritePosition());
    device->setPresenceDetected(response.isPresenceDetected());
    device->setTwilight(response.isTwilight());
    RESET_REFRESH_TIME(device);
}
void DeltaDoreX2Driver::onLightInfoResponse(LightInfoResponse& response)
{
    DeltaDoreDeviceInfo* device = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());
    setContextResponseClass(LightInfoResponse_t);
    device->setDeviceType(DeltaDoreDeviceInfo::DEVICE_TYPE_LIGHT);
    device->setOnLine(true);
    device->setLastResponseStatus(response.getStatus());

    device->setChannelCount(response.getChannelCount());
    device->setLightActuatorType(response.getActuatorType());
    device->setMulticolor(response.isMulticolor());
}
void DeltaDoreX2Driver::onLightColorResponse(LightColorResponse& response)
{
    DeltaDoreDeviceInfo* device = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());
    setContextResponseClass(LightInfoResponse_t);
    device->setDeviceType(DeltaDoreDeviceInfo::DEVICE_TYPE_LIGHT);
    device->setOnLine(true);
    device->setLastResponseStatus(response.getStatus());

    device->setColorRed(response.getRedValue());
    device->setColorBlue(response.getBlueValue());
    device->setColorGreen(response.getGreenValue());
    RESET_REFRESH_TIME(device);
}


void DeltaDoreX2Driver::nodeDiscovered(const NodeDiscoveredEvent& evt)
{
    printf("In network %d found node %d\n", evt.getNetwork()->getIdentifier(), evt.getNode().toInt());
    setContextRequestNetwork(evt.getNetwork()->getIdentifier());
    setContextRequestNode(evt.getNode().toInt());
}

int DeltaDoreX2Driver::tryToGetDeviceType(int network, int node)
{
    logger(LEVEL_DEBUG, "Query device type of node %d.%d", network, node);
    LOCK_CMD_HANDLING;

    MeshController* mCtrl = controller->convert<MeshController*>();
    Network* pNetwork = mCtrl->getNetwork(network);
    acked = false;
    setContextRequestNetwork(network);
    setContextRequestNode(node);

    // try if it was light
    Request* pRequest = createRequest(LightInfoRequest_t);
    pRequest->setNetwork(pNetwork);
    pRequest->addNode(Node::valueOf(node), LightCommandArg::NA);
    beginTransaction(pRequest);

    waitAck(30);
    DeltaDoreDeviceInfo* pDevice = getDeviceInfo(network, node);
    if (pDevice->getLastResponseStatus() == ResponseStatus::OK){
        UNLOCK_CMD_HANDLING;
        delete pRequest;
        UPDATE_REFRESH_TIME(pDevice);
        return DeltaDoreDeviceInfo::DEVICE_TYPE_LIGHT;
    }
    if (pDevice->getLastResponseStatus() != ResponseStatus::NOT_CAPABLE_NODE){
        UNLOCK_CMD_HANDLING;
        delete pRequest;
        UPDATE_DETECT_TIME(pDevice);
        return DeltaDoreDeviceInfo::DEVICE_TYPE_UNKNOWN;
    }

    // try if it was roller shutter
    pRequest = createRequest(RollerShutterInfoRequest_t);
    pRequest->setNetwork(pNetwork);
    pRequest->addNode(Node::valueOf(node), RollerShutterCommandArg::NA);
    beginTransaction(pRequest);

    waitAck(30);
    pDevice = getDeviceInfo(network, node);
    if (pDevice->getLastResponseStatus() == ResponseStatus::OK){
        UNLOCK_CMD_HANDLING;
        delete pRequest;
        UPDATE_REFRESH_TIME(pDevice);
        return DeltaDoreDeviceInfo::DEVICE_TYPE_ROLLER_SHUTTER;
    }

    // any try is failed
    UNLOCK_CMD_HANDLING;
    delete pRequest;
    RESET_REFRESH_TIME(pDevice);
    return DeltaDoreDeviceInfo::DEVICE_TYPE_UNKNOWN;
}

cJSON* DeltaDoreX2Driver::debugPrintRead(bool enablePrint)
{
    cJSON* root=cJSON_CreateObject();
//    ((CoreController*)controller)->m_isPrintReadByte = enablePrint;
    CoreController* pCtrl = controller->convert<CoreController*>();
    pCtrl->m_isPrintReadByte = enablePrint;
    SIMPLE_SUCCESS_RESPONSE(root);
    return root;
}
cJSON* DeltaDoreX2Driver::refreshTopology()
{
     MeshController* meshctrl = controller->convert<MeshController*>();
    int nn = meshctrl->getNetworkCount();
    for(int i = 0; i < nn; i++)
    {
        Network* pNetwork = meshctrl->getNetwork(i);
        if (pNetwork == NULL)
        {
            continue;
        }

        vector<Node> nodes = pNetwork->getTopology();
        vector<Node>::iterator it;
        for(it=nodes.begin(); it != nodes.end(); it++)
        {
            Node node = *it;
            int deviceType = tryToGetDeviceType(pNetwork->getIdentifier(), node.toInt());
            switch (deviceType){
            case DeltaDoreDeviceInfo::DEVICE_TYPE_LIGHT:
                printf("Node %d in network %d is %s\n", node.toInt(), i, DEVICE_TYPE_NAME_LIGHT);
                break;
            case DeltaDoreDeviceInfo::DEVICE_TYPE_ROLLER_SHUTTER:
                printf("Node %d in network %d is %s\n", node.toInt(), i, DEVICE_TYPE_NAME_ROLLERSHUTTER);
                break;
            default:
                printf("Node %d in network %d is %s\n", node.toInt(), i, DEVICE_TYPE_NAME_UNKNOWN);
            }
        }
    }
    return getTopology();
}

/**
 * return JSON format should be like this:
 * {   "message":"success"|"<any other message>",
 *     "success": true | false,
 *     "data": {
 *          "<network id>" : [ {<node x>, <node y}],
 *          "<network id>" : [ {<node x>, <node y}]
 *     }
 * }
 *
 */
cJSON* DeltaDoreX2Driver::getTopology()
{
    char buffer[10+MAX_NAME_LENGTH];
    cJSON* root=cJSON_CreateObject();

    MeshController* ctrl = controller->convert<MeshController*>();
    int nn = ctrl->getNetworkCount();
    if (nn == 0)
    {
        SIMPLE_ERROR_RESPONSE(root, "No any network");
        return root;
    }

    SIMPLE_SUCCESS_RESPONSE(root);

    cJSON* pResponseData=cJSON_CreateObject();
    cJSON_AddItemToObject(root, "data", pResponseData);


    DeltaDoreDeviceInfo* pDevice;
    for(int i = 0; i < nn; i++)
    {
        Network* pNetwork = ctrl->getNetwork(i);
        if (pNetwork == NULL)
        {
            continue;
        }
        vector<Node> nodes = pNetwork->getTopology();
        vector<Node>::iterator it;
        int networkId =  pNetwork->getIdentifier();

        // if no name, and no nodes, skip it
        if (nodes.size() <=0){
            continue;
        }
        snprintf(buffer, sizeof(buffer)-1, "%d",networkId);

        // save name first
        cJSON* pNetWorkInfo=cJSON_CreateObject();
        cJSON_AddItemToObject(pResponseData, buffer, pNetWorkInfo);

        // save nodes then
        cJSON* pNetWorkData=cJSON_CreateArray();
        cJSON_AddItemToObject(pNetWorkInfo, "nodes", pNetWorkData);


        for(it=nodes.begin(); it != nodes.end(); it++)
        {
            Node node = *it;
            snprintf(buffer, sizeof(buffer)-1, "%d", node.toInt());
            cJSON* pNodeData=cJSON_CreateObject();
            cJSON_AddNumberToObject(pNodeData, "id", node.toInt());
            pDevice = getDeviceInfo(networkId, node.toInt());
            switch (pDevice->getDeviceType()){
            case DeltaDoreDeviceInfo::DEVICE_TYPE_LIGHT:
                cJSON_AddStringToObject(pNodeData, "type", DEVICE_TYPE_NAME_LIGHT);
                cJSON_AddNumberToObject(pNodeData, "level", pDevice->getLevel());
				cJSON_AddNumberToObject(pNodeData, "colorRed", pDevice->getColorRed() & 0xFF);
				cJSON_AddNumberToObject(pNodeData, "colorGreen", pDevice->getColorGreen() & 0xFF);
				cJSON_AddNumberToObject(pNodeData, "colorBlue", pDevice->getColorBlue() & 0xFF);
				cJSON_AddNumberToObject(pNodeData, "online", pDevice->getOnLine() & 0xFF);
                break;
            case DeltaDoreDeviceInfo::DEVICE_TYPE_ROLLER_SHUTTER:
                cJSON_AddStringToObject(pNodeData, "type", DEVICE_TYPE_NAME_ROLLERSHUTTER);
                cJSON_AddNumberToObject(pNodeData, "position", pDevice->getPosition());
                cJSON_AddNumberToObject(pNodeData, "online", pDevice->getOnLine() & 0xFF);
                break;
            default:
                cJSON_AddStringToObject(pNodeData, "type", DEVICE_TYPE_NAME_UNKNOWN);
                cJSON_AddNumberToObject(pNodeData, "online", pDevice->getOnLine() & 0xFF);
            }

            const char* pDeviceName = pDevice->getName();
            cJSON_AddStringToObject(pNodeData, "name", pDeviceName[0]==0?CONST_NO_NAME:pDeviceName);
            pDeviceName = pDevice->getRegion();
            cJSON_AddStringToObject(pNodeData, "region", pDeviceName[0]==0?CONST_NO_NAME:pDeviceName);
            cJSON_AddItemToArray(pNetWorkData, pNodeData);

        }
    }
    return root;
}


cJSON* DeltaDoreX2Driver::deleteNode(int network, int node)
{
    LOCK_CMD_HANDLING;
    cJSON* root=cJSON_CreateObject();
    char errMsg[100];
    Network* net = checkNetwork(network, errMsg, sizeof(errMsg));
    if (net == NULL){
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, errMsg);
        return root;
    }
    if (!checkNode(net, node, errMsg, sizeof(errMsg))){
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, errMsg);
        return root;
    }

    Node tgtNode = Node::valueOf(node);
    trans_end_flag = false;
    net->deleteNode(tgtNode, true);
    waitAck(5);


    if (!acked)
    {
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, "Deleted Node  has no response");
        return root;
    }

    UNLOCK_CMD_HANDLING;
    SIMPLE_SUCCESS_RESPONSE(root);
    return root;
}


cJSON* DeltaDoreX2Driver::registerNode(int network)
{
    LOCK_CMD_HANDLING;
    cJSON* root=cJSON_CreateObject();
    char errMsg[100];
    Network* pNetwork = checkNetwork(network, errMsg, sizeof(errMsg));
    if (pNetwork == NULL){
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, errMsg);
        return root;
    }
    setContextRequestNetwork(network);
    setContextRequestNode(-1);
    acked = false;
    trans_end_flag = false;
    pNetwork->startNodeDiscovery(true);
    waitAck(30);


    if (!acked)
    {
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, "Negative response");
        return root;
    }

    int node = getContextRequestNode();
    if (node < 0){
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, "No response");
        return root;
    }

    UNLOCK_CMD_HANDLING;
    SIMPLE_SUCCESS_RESPONSE(root);
    cJSON* pData=cJSON_CreateObject();
    cJSON_AddItemToObject(root, "data", pData);
    cJSON_AddNumberToObject(pData, "network", network);
    cJSON_AddNumberToObject(pData, "node", node);
    int deviceType = tryToGetDeviceType(network, node);
    DeltaDoreDeviceInfo* device = getDeviceInfo(network, node);
    switch (deviceType){
    case DeltaDoreDeviceInfo::DEVICE_TYPE_LIGHT:
        cJSON_AddStringToObject(pData, "responseStatus", device->getLastResponseStatus().toString().c_str());
		cJSON_AddStringToObject(pData, "type", DEVICE_TYPE_NAME_LIGHT);
        cJSON_AddNumberToObject(pData, "level", device->getLevel());
        cJSON_AddBoolToObject(pData, "overloadFaulty", device->isOverloadFaulty());
        cJSON_AddBoolToObject(pData, "commandFaulty", device->isCommandFaulty());
        cJSON_AddBoolToObject(pData, "overheatingFaulty", device->isOverheatingFaulty());
        cJSON_AddBoolToObject(pData, "favoritePosition", device->isFavoritePosition());
        cJSON_AddBoolToObject(pData, "presenceDetected", device->isPresenceDetected());
        cJSON_AddBoolToObject(pData, "twilight", device->isTwilight());
        break;
    case DeltaDoreDeviceInfo::DEVICE_TYPE_ROLLER_SHUTTER:
        cJSON_AddStringToObject(pData, "responseStatus", device->getLastResponseStatus().toString().c_str());
		cJSON_AddStringToObject(pData, "type", DEVICE_TYPE_NAME_ROLLERSHUTTER);
        cJSON_AddNumberToObject(pData, "position", device->getPosition());
        cJSON_AddBoolToObject(pData, "favoritePosition", device->isFavoritePosition());
        cJSON_AddBoolToObject(pData, "intrusionDetected", device->isIntrusionDetected());
        cJSON_AddBoolToObject(pData, "loweringFaulty", device->isLoweringFaulty());
        cJSON_AddBoolToObject(pData, "obstacleFaulty", device->isObstacleFaulty());
        cJSON_AddBoolToObject(pData, "overheatingFaulty", device->isOverheatingFaulty());
        cJSON_AddBoolToObject(pData, "raisingFaulty", device->isRaisingFaulty());
        break;
    default:
        SIMPLE_ERROR_RESPONSE(root, "unsupported device type");
        break;
    }

    return root;
}

cJSON* DeltaDoreX2Driver::queryRollerShutterStatus(int network, int node)
{
    cJSON* root=cJSON_CreateObject();
    PREPARE_REQUEST(root, network, node);
    LOCK_CMD_HANDLING;
    acked = false;
    setContextRequestNetwork(network);
    setContextRequestNode(node);

    Request *req = createRequest(RollerShutterStatusRequest_t);
    req->setNetwork(net);
    req->addNode(Node::valueOf(node), RollerShutterCommandArg::NA);

    beginTransaction(req);
    waitAck(5);
    delete req;


    if (!acked)
    {
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, "Response negative");
        return root;
    }

//    SIMPLE_SUCCESS_RESPONSE(root);
//    if (getContextResponseClass() != RollerShutterStatusResponse_t)
//    {
//        SIMPLE_ERROR_RESPONSE(root, "No response");
//        return root;
//    }

    DeltaDoreDeviceInfo* device = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());
    ResponseStatus status = device->getLastResponseStatus();
    if (status != ResponseStatus::OK)
    {
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, status.toString().c_str());
        return root;
    }
    UNLOCK_CMD_HANDLING;

    SIMPLE_SUCCESS_RESPONSE(root);
    cJSON* pData=cJSON_CreateObject();
    cJSON_AddItemToObject(root, "data", pData);

    cJSON_AddStringToObject(pData, "responseStatus", device->getLastResponseStatus().toString().c_str());
    cJSON_AddNumberToObject(pData, "position", device->getPosition());
    cJSON_AddBoolToObject(pData, "favoritePosition", device->isFavoritePosition());
    cJSON_AddBoolToObject(pData, "intrusionDetected", device->isIntrusionDetected());
    cJSON_AddBoolToObject(pData, "loweringFaulty", device->isLoweringFaulty());
    cJSON_AddBoolToObject(pData, "obstacleFaulty", device->isObstacleFaulty());
    cJSON_AddBoolToObject(pData, "overheatingFaulty", device->isOverheatingFaulty());
    cJSON_AddBoolToObject(pData, "raisingFaulty", device->isRaisingFaulty());
    return root;

}

cJSON* DeltaDoreX2Driver::queryRollerShutterInfo(int network, int node)
{

    cJSON* root=cJSON_CreateObject();
    PREPARE_REQUEST(root, network, node);

    LOCK_CMD_HANDLING;
    acked = false;
    setContextRequestNetwork(network);
    setContextRequestNode(node);

    Request *req = createRequest(RollerShutterInfoRequest_t);
    req->setNetwork(net);
    req->addNode(Node::valueOf(node), RollerShutterCommandArg::NA);

    beginTransaction(req);
    waitAck(5);
    delete req;


    if (!acked)
    {
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, "Response negative");
        return root;
    }

//    SIMPLE_SUCCESS_RESPONSE(root);
//    if (getContextResponseClass() != RollerShutterStatusResponse_t)
//    {
//        SIMPLE_ERROR_RESPONSE(root, "No response");
//        return root;
//    }

    DeltaDoreDeviceInfo* device = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());
    ResponseStatus status = device->getLastResponseStatus();
    if (status != ResponseStatus::OK)
    {
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, status.toString().c_str());
        return root;
    }
    UNLOCK_CMD_HANDLING;

    SIMPLE_SUCCESS_RESPONSE(root);
    cJSON* pData=cJSON_CreateObject();
    cJSON_AddItemToObject(root, "data", pData);

    cJSON_AddStringToObject(pData, "responseStatus", device->getLastResponseStatus().toString().c_str());
    cJSON_AddNumberToObject(pData, "channelCount", device->getChannelCount());
    cJSON_AddStringToObject(pData, "actuatorType", device->getRollerShutterActuatorType().toString().c_str());

    return root;

}

cJSON* DeltaDoreX2Driver::controlRollerShutter(int network, int node, const RollerShutterCommandArg& action)
{
    cJSON* root=cJSON_CreateObject();
    PREPARE_REQUEST(root, network, node);

    LOCK_CMD_HANDLING;
    acked = false;
    setContextRequestNetwork(network);
    setContextRequestNode(node);

    Request *req = createRequest(RollerShutterCommandRequest_t);
    req->setNetwork(net);
    req->addNode(Node::valueOf(node), action);

    beginTransaction(req);
    waitAck(5);
    delete req;

    if (!acked)
    {
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, "RollerShutter No response");
        return root;
    }
    DeltaDoreDeviceInfo* device = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());
    UNLOCK_CMD_HANDLING;
    UPDATE_REFRESH_TIME(device);

    ResponseStatus status = device->getLastResponseStatus();
    if (status != ResponseStatus::OK)
    {
        SIMPLE_ERROR_RESPONSE(root, status.toString().c_str());
        return root;
    }

    SIMPLE_SUCCESS_RESPONSE(root);
    return root;
}

cJSON* DeltaDoreX2Driver::queryLightStatus(int network, int node)
{
    cJSON* root=cJSON_CreateObject();
    PREPARE_REQUEST(root, network, node);

    LOCK_CMD_HANDLING;
    acked = false;
    setContextRequestNetwork(network);
    setContextRequestNode(node);

    Request *req = createRequest(LightStatusRequest_t);
    req->setNetwork(net);
    req->addNode(Node::valueOf(node), LightCommandArg::NA);

    beginTransaction(req);
    waitAck(5);
    delete req;


    if (!acked)
    {
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, "Response negative");
        return root;
    }

    DeltaDoreDeviceInfo* device = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());
    ResponseStatus status = device->getLastResponseStatus();
    if (status != ResponseStatus::OK)
    {
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, status.toString().c_str());
        return root;
    }

    SIMPLE_SUCCESS_RESPONSE(root);
    cJSON* pData=cJSON_CreateObject();
    cJSON_AddItemToObject(root, "data", pData);

    cJSON_AddStringToObject(pData, "responseStatus", device->getLastResponseStatus().toString().c_str());
    cJSON_AddNumberToObject(pData, "level", device->getLevel());
    cJSON_AddBoolToObject(pData, "overloadFaulty", device->isOverloadFaulty());
    cJSON_AddBoolToObject(pData, "commandFaulty", device->isCommandFaulty());
    cJSON_AddBoolToObject(pData, "overheatingFaulty", device->isOverheatingFaulty());
    cJSON_AddBoolToObject(pData, "favoritePosition", device->isFavoritePosition());
    cJSON_AddBoolToObject(pData, "presenceDetected", device->isPresenceDetected());
    cJSON_AddBoolToObject(pData, "twilight", device->isTwilight());

    UNLOCK_CMD_HANDLING;
    return root;

}

cJSON* DeltaDoreX2Driver::queryLightInfo(int network, int node)
{

    cJSON* root=cJSON_CreateObject();
    PREPARE_REQUEST(root, network, node);

    LOCK_CMD_HANDLING;
    acked = false;
    setContextRequestNetwork(network);
    setContextRequestNode(node);

    Request *req = createRequest(LightInfoRequest_t);
    req->setNetwork(net);
    req->addNode(Node::valueOf(node), LightCommandArg::NA);

    beginTransaction(req);
    waitAck(5);
    delete req;

    if (!acked)
    {
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, "LightInfo Response negative");
        return root;
    }

    DeltaDoreDeviceInfo* device = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());
    UNLOCK_CMD_HANDLING;

    ResponseStatus status = device->getLastResponseStatus();
    if (status != ResponseStatus::OK)
    {
        SIMPLE_ERROR_RESPONSE(root, status.toString().c_str());
        return root;
    }

    SIMPLE_SUCCESS_RESPONSE(root);
    cJSON* pData=cJSON_CreateObject();
    cJSON_AddItemToObject(root, "data", pData);

    cJSON_AddStringToObject(pData, "responseStatus", device->getLastResponseStatus().toString().c_str());
    cJSON_AddNumberToObject(pData, "channelCount", device->getChannelCount());
    cJSON_AddStringToObject(pData, "actuatorType", device->getLightActuatorType().toString().c_str());
    cJSON_AddBoolToObject(pData, "multicolor", device->isMulticolor());

    return root;

}

cJSON* DeltaDoreX2Driver::controlLight(int network, int node, const LightCommandArg& action)
{

    cJSON* root=cJSON_CreateObject();
    PREPARE_REQUEST(root, network, node);

    LOCK_CMD_HANDLING;
    acked = false;
    setContextRequestNetwork(network);
    setContextRequestNode(node);

    Request *req = createRequest(LightCommandRequest_t);
    req->setNetwork(net);
    req->addNode(Node::valueOf(node), action);

    beginTransaction(req);
    waitAck(5);
    delete req;

    if (!acked)
    {
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, "ControlLight Response negative");
        return root;
    }
    DeltaDoreDeviceInfo* device = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());
    UNLOCK_CMD_HANDLING;

    UPDATE_REFRESH_TIME(device);
    ResponseStatus status = device->getLastResponseStatus();
    if (status != ResponseStatus::OK)
    {
        SIMPLE_ERROR_RESPONSE(root, status.toString().c_str());
        return root;
    }

    SIMPLE_SUCCESS_RESPONSE(root);
    return root;
}

cJSON* DeltaDoreX2Driver::setLightLevel(int network, int node, int level)
{
    int correctLevel = level;
    if (level < 0){
        correctLevel = 0;
    }else if (level > 100){
        correctLevel = 100;
    }
    return controlLight(network, node, LightCommandArg::percent(correctLevel));
}


#define CHECK_COLOR(root, c)                                             \
    if ((c) < 0 || (c) > 255){                                           \
        cJSON_AddNumberToObject(root, "input " #c, c);                   \
        SIMPLE_ERROR_RESPONSE(root, "Color " #c " should be [0,255]");   \
        return root;                                                     \
    }

cJSON* DeltaDoreX2Driver::setLightColor(int network, int node, int red, int green, int blue)
{

    cJSON* root=cJSON_CreateObject();
    CHECK_COLOR(root, red);
    CHECK_COLOR(root, green);
    CHECK_COLOR(root, blue);

    PREPARE_REQUEST(root, network, node);
    LOCK_CMD_HANDLING;
    acked = false;
    setContextRequestNetwork(network);
    setContextRequestNode(node);

    Request *req = createRequest(LightSetColorRequest_t);
    req->setNetwork(net);
    req->addNode(Node::valueOf(node), new LightColorArg(red, green, blue));

    beginTransaction(req);
    waitAck(5);
    delete req;

    if (!acked)
    {
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, "setLightColor Response negative");
        return root;
    }
    DeltaDoreDeviceInfo* device = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());
    UNLOCK_CMD_HANDLING;

    UPDATE_REFRESH_TIME(device);
    ResponseStatus status = device->getLastResponseStatus();
    if (status != ResponseStatus::OK)
    {
        SIMPLE_ERROR_RESPONSE(root, status.toString().c_str());
        return root;
    }

    SIMPLE_SUCCESS_RESPONSE(root);
    return root;
}
#undef CHECK_COLOR

cJSON* DeltaDoreX2Driver::queryLightColor(int network, int node)
{
    cJSON* root=cJSON_CreateObject();
    PREPARE_REQUEST(root, network, node);

    LOCK_CMD_HANDLING;
    acked = false;
    setContextRequestNetwork(network);
    setContextRequestNode(node);

    Request *req = createRequest(LightGetColorRequest_t);
    req->setNetwork(net);
    req->addNode(Node::valueOf(node), NodeArg::NONE);

    beginTransaction(req);
    waitAck(5);
    delete req;

    if (!acked)
    {
        UNLOCK_CMD_HANDLING;
        SIMPLE_ERROR_RESPONSE(root, "queryLightColor Response negative");
        return root;
    }

    DeltaDoreDeviceInfo* device = getDeviceInfo(getContextRequestNetwork(), getContextRequestNode());
    UNLOCK_CMD_HANDLING;

    ResponseStatus status = device->getLastResponseStatus();
    if (status != ResponseStatus::OK)
    {
        SIMPLE_ERROR_RESPONSE(root, status.toString().c_str());
        return root;
    }

    SIMPLE_SUCCESS_RESPONSE(root);
    cJSON* pData=cJSON_CreateObject();
    cJSON_AddItemToObject(root, "data", pData);

    cJSON_AddStringToObject(pData, "responseStatus", device->getLastResponseStatus().toString().c_str());
    cJSON_AddNumberToObject(pData, "colorRed", device->getColorRed() & 0xFF);
    cJSON_AddNumberToObject(pData, "colorGreen", device->getColorGreen() & 0xFF);
    cJSON_AddNumberToObject(pData, "colorBlue", device->getColorBlue() & 0xFF);

    return root;
}


cJSON* DeltaDoreX2Driver::setNodeName(int network, int node, const char* region, const char* name)
{
    cJSON* root=cJSON_CreateObject();
    PREPARE_REQUEST(root, network, node);

    DeltaDoreDeviceInfo* device = getDeviceInfo(network, node);

    if (strcmp(CONST_NO_NAME, region) != 0 || strcmp(CONST_NO_NAME, name) != 0) {
        device->setRegion(region);
        device->setName(name);
    }else{
        device->setRegion("");
        device->setName("");
    }
    UPDATE_REFRESH_TIME(device);
    SIMPLE_SUCCESS_RESPONSE(root);
    return root;
}

cJSON* DeltaDoreX2Driver::setNodeDeviceType(int network, int node, const char* deviceType){
    cJSON* root=cJSON_CreateObject();
    PREPARE_REQUEST(root, network, node);
    // TODO debug
    printf("set device (%d.%d) type to %s\n", network, node, deviceType);
    DeltaDoreDeviceInfo* device = getDeviceInfo(network, node);

    if (strcmp("light", deviceType) == 0){
        device->setDeviceType(DeltaDoreDeviceInfo::DEVICE_TYPE_LIGHT);
        device->setLightActuatorType(LightActuatorType::BINARY_STATE);
    }else if (strcmp("dimmer", deviceType) == 0){
        device->setDeviceType(DeltaDoreDeviceInfo::DEVICE_TYPE_LIGHT);
        device->setLightActuatorType(LightActuatorType::STATEFULL);
    }else if (strcmp("rollerShutter", deviceType) == 0){
        device->setDeviceType(DeltaDoreDeviceInfo::DEVICE_TYPE_ROLLER_SHUTTER);
        device->setRollerShutterActuatorType(RollerShutterActuatorType::TERNARY_STATE);
    }else{
        device->setDeviceType(DeltaDoreDeviceInfo::DEVICE_TYPE_UNKNOWN);
    }
    UPDATE_REFRESH_TIME(device);
    SIMPLE_SUCCESS_RESPONSE(root);
    return root;

}

/**********************************************************************************************/

void fillRollerShutterInfo(DevicePhysicalState* pPhyState, DeltaDoreDeviceInfo* pDeviceInfo){
    pPhyState->nextRefreshTs = pDeviceInfo->getNextRefreshTs();
    pPhyState->deviceType = DEVICE_ROLLER_SHUTTER;
    pPhyState->simpleOnOff = pDeviceInfo->getRollerShutterActuatorType() == RollerShutterActuatorType::TERNARY_STATE;
    pPhyState->position = pDeviceInfo->getPosition();
    pPhyState->channelCount = pDeviceInfo->getChannelCount();
    pPhyState->region = pDeviceInfo->getRegion();
    pPhyState->name = pDeviceInfo->getName();
}
void fillLightInfo(DevicePhysicalState* pPhyState, DeltaDoreDeviceInfo* pDeviceInfo){
    pPhyState->nextRefreshTs = pDeviceInfo->getNextRefreshTs();
    pPhyState->deviceType = DEVICE_LIGHT;
    pPhyState->simpleOnOff = pDeviceInfo->getLightActuatorType() == LightActuatorType::BINARY_STATE;
    pPhyState->level = 0xff & (pDeviceInfo->getLevel()*255/100);
    pPhyState->redColor = pDeviceInfo->getColorRed();
    pPhyState->greenColor = pDeviceInfo->getColorGreen();
    pPhyState->blueColort = pDeviceInfo->getColorBlue();
    pPhyState->channelCount = pDeviceInfo->getChannelCount();
    pPhyState->multiColor = pDeviceInfo->isMulticolor();
    pPhyState->region = pDeviceInfo->getRegion();
    pPhyState->name = pDeviceInfo->getName();
}
void DeltaDoreX2Driver::getAllDevicePhyStates(list<DevicePhysicalState*>& deviceStateList)
{
    MeshController* meshctrl = controller->convert<MeshController*>();
    int newDeviceType;
    int nn = meshctrl->getNetworkCount();
    for(int i = 0; i < nn; i++)
    {
        Network* pNetwork = meshctrl->getNetwork(i);
        if (pNetwork == NULL)
        {
            continue;
        }

        vector<Node> nodes = pNetwork->getTopology();
        vector<Node>::iterator it;
        for(it=nodes.begin(); it != nodes.end(); it++)
        {
            Node node = *it;
            int networkId = pNetwork->getIdentifier();
            int nodeId = node.toInt();
            DeltaDoreDeviceInfo* pDeviceInfo = getDeviceInfo(networkId,nodeId);
            DevicePhysicalState* pPhyState = new DevicePhysicalState();
            pPhyState->network = networkId;
            pPhyState->node = nodeId;


            int deviceType = pDeviceInfo->getDeviceType();
            pPhyState->deviceType = deviceType;
            pPhyState->online = pDeviceInfo->getOnLine();
            //printf("PHY query: (%d.%d) type=%d, online=%d\n", i, node.toInt(), deviceType, pDeviceInfo->getOnLine());
//            if (!pDeviceInfo->getOnLine()){
//                logger(LEVEL_INFO, "fill UNKNOWN to node %d.%d because cannot connect", i, node.toInt());
//                pPhyState->nextRefreshTs = pDeviceInfo->getNextRefreshTs();
//                pPhyState->region = pDeviceInfo->getRegion();
//                pPhyState->name = pDeviceInfo->getName();
//                pPhyState->online = false;
//                pPhyState->simpleOnOff = pDeviceInfo->isS;
//                deviceStateList.push_back(pPhyState);
//                continue;
//            }
            switch (deviceType){
            case DeltaDoreDeviceInfo::DEVICE_TYPE_LIGHT:
                //logger(LEVEL_INFO, "fill LightInfo to node %d.%d", node.toInt(), i);
                fillLightInfo(pPhyState, pDeviceInfo);
                deviceStateList.push_back(pPhyState);
                break;
            case DeltaDoreDeviceInfo::DEVICE_TYPE_ROLLER_SHUTTER:
                //logger(LEVEL_INFO, "fill RollerShutterInfo to node %d.%d", node.toInt(), i);
                fillRollerShutterInfo(pPhyState, pDeviceInfo);
                deviceStateList.push_back(pPhyState);
                break;
            default:
                // 如果底层通讯失败，并且未设置过设备类型，那么这个设备就是未知设备。
                // 未知设备类型数据格式为：online = false
                //logger(LEVEL_INFO, "fill UNKNOWN to node %d.%d", i, node.toInt());
                pPhyState->nextRefreshTs = pDeviceInfo->getNextRefreshTs();
                pPhyState->region = pDeviceInfo->getRegion();
                pPhyState->name = pDeviceInfo->getName();
                pPhyState->deviceType = DEVICE_UNKNOWN;
                pPhyState->online = false;
                pPhyState->simpleOnOff = true;
                deviceStateList.push_back(pPhyState);
                break;
            }
        }
    }
}

int DeltaDoreX2Driver::queryRoolershutterPhyState(int network, int node, DevicePhysicalState& result)
{
    result.network = network;
    result.node = node;
    char errMsg[100];
    Network* net = checkNetwork(network, errMsg, sizeof(errMsg));
    if (net == NULL){
        return 0;
    }
    if (!checkNode(net, node, errMsg, sizeof(errMsg))){
        return 0;
    }
    LOCK_CMD_HANDLING;
    setContextRequestNetwork(network);
    setContextRequestNode(node);
    // query status first
    Request *req = createRequest(RollerShutterStatusRequest_t);
    req->setNetwork(net);
    req->addNode(Node::valueOf(node), RollerShutterCommandArg::NA);

    beginTransaction(req);
    waitAck(5);
    delete req;
    DeltaDoreDeviceInfo* device = getDeviceInfo(network, node);
    ResponseStatus status = device->getLastResponseStatus();
    if (status == ResponseStatus::NOT_CAPABLE_NODE){
        UNLOCK_CMD_HANDLING;
        return 2; // should query real type
    }
    if (status != ResponseStatus::OK)
    {
        result.online = false;
    }

    // query info next
    req = createRequest(RollerShutterInfoRequest_t);
    req->setNetwork(net);
    req->addNode(Node::valueOf(node), RollerShutterCommandArg::NA);

    beginTransaction(req);
    waitAck(5);
    delete req;
    UNLOCK_CMD_HANDLING;
    status = device->getLastResponseStatus();
    if (status != ResponseStatus::OK)
    {
        result.online = false;
    }
    DeltaDoreDeviceInfo* pDeviceInfo = getDeviceInfo(network,node);
    fillRollerShutterInfo(&result, pDeviceInfo);

    return 1;
}

int DeltaDoreX2Driver::queryLightPhyState(int network, int node, DevicePhysicalState& result)
{
    result.network = network;
    result.node = node;
    char errMsg[100];
    Network* net = checkNetwork(network, errMsg, sizeof(errMsg));
    if (net == NULL){
        return 0;
    }
    if (!checkNode(net, node, errMsg, sizeof(errMsg))){
        return 0;
    }
    LOCK_CMD_HANDLING;
    setContextRequestNetwork(network);
    setContextRequestNode(node);
    // query status first
    Request *req = createRequest(LightStatusRequest_t);
    req->setNetwork(net);
    req->addNode(Node::valueOf(node), LightCommandArg::NA);

    beginTransaction(req);
    waitAck(5);
    delete req;
    DeltaDoreDeviceInfo* device = getDeviceInfo(network, node);

    ResponseStatus status = device->getLastResponseStatus();
    if (status == ResponseStatus::NOT_CAPABLE_NODE){
        UNLOCK_CMD_HANDLING;
        return 2; // should query real type
    }
    if (status != ResponseStatus::OK)
    {
        result.online = false;
    }
    // query info next
    req = createRequest(LightInfoRequest_t);
    req->setNetwork(net);
    req->addNode(Node::valueOf(node), LightCommandArg::NA);

    beginTransaction(req);
    waitAck(5);
    delete req;
    UNLOCK_CMD_HANDLING;
    status = device->getLastResponseStatus();
    if (status != ResponseStatus::OK)
    {
        result.online = false;
    }
    DeltaDoreDeviceInfo* pDeviceInfo = getDeviceInfo(network,node);
    fillLightInfo(&result, pDeviceInfo);

    return 1;
}

bool DeltaDoreX2Driver::getNodeRealType(int network, int node, DevicePhysicalState& result)
{
    int realType = tryToGetDeviceType(network, node);
    result.network = network;
    result.node = node;
    DeltaDoreDeviceInfo* pDeviceInfo = getDeviceInfo(network,node);
    switch (realType){
        case DeltaDoreDeviceInfo::DEVICE_TYPE_LIGHT:
            fillLightInfo(&result, pDeviceInfo);
            result.online = true;
            break;
        case DeltaDoreDeviceInfo::DEVICE_TYPE_ROLLER_SHUTTER:
            fillRollerShutterInfo(&result, pDeviceInfo);
            result.online = true;
            break;
        default:
            return false;
    }
    return true;
}

