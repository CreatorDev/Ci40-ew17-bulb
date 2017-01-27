#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <awa/client.h>
#include <awa/common.h>
#include "log.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define OPERATION_PERFORM_TIMEOUT 1000
#define LIGHT_CONTROL_OBJECT_ID 3311
#define COLOUR_RESOURCE_ID 5706

FILE *g_debugStream = NULL;
int g_debugLevel = LOG_INFO;
static bool _keepRunning;

static const char *_validColours[] = {
    "red", "green", "blue", "yellow"};

static void exitApp(int __attribute__((unused)) (signo))
{
    LOG(LOG_INFO, "Exitting app...");
    _keepRunning = false;
}

static void setBulbColour(const char *colour)
{
    //ToDo implement
}

static void colorChangeCallback(const AwaChangeSet *changeSet, void *context)
{
    const char *value;
    AwaChangeSet_GetValueAsCStringPointer(changeSet, "/3311/0/5706", &value);
    int i = 0;
    bool validColour = false;
    for (i = 0; i < 4; i++)
    {
        if (strcmp(_validColours[i], value) == 0)
        {
            validColour = true;
            break;
        }
    }
    if (validColour)
    {
        LOG(LOG_INFO, "Value of resource /3311/0/5706 changed to: %s\n", value);
        setBulbColour(value);
    }
    else
    {
        LOG(LOG_WARN, "Value of resource /3311/0/5706 changed to: %s but it is invalid colour.", value);
    }
}

int main(int argc, char **argv)
{
    _keepRunning = true;

    struct sigaction action = {
        .sa_handler = exitApp,
        .sa_flags = 0};

    if (sigemptyset(&action.sa_mask) < 0 || sigaction(SIGINT, &action, NULL) < 0)
    {
        LOG(LOG_ERR, "Failed to set Control+C handler\n");
        return -1;
    }

    AwaClientSession *session = AwaClientSession_New();

    AwaClientSession_Connect(session);

    // Define Lightcontrol IPSO object if not exists
    if (AwaClientSession_IsObjectDefined(session, 3311) == false)
    {
        AwaObjectDefinition *objectDefinition = AwaObjectDefinition_New(LIGHT_CONTROL_OBJECT_ID, "LightControl", 0, 1);
        AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, COLOUR_RESOURCE_ID, "Colour", false, AwaResourceOperations_ReadWrite, NULL);
        AwaClientDefineOperation *operationDefine = AwaClientDefineOperation_New(session);
        AwaClientDefineOperation_Add(operationDefine, objectDefinition);
        AwaClientDefineOperation_Perform(operationDefine, OPERATION_PERFORM_TIMEOUT);
        AwaClientDefineOperation_Free(&operationDefine);
    }

    // Create LightControl IPSO object instance
    char lightControlObjectInstancePath[40];
    char colorResourcePath[40];
    sprintf(&lightControlObjectInstancePath[0], "/%d/%d", LIGHT_CONTROL_OBJECT_ID, 0);
    sprintf(&colorResourcePath[0], "/%d/%d/%d", LIGHT_CONTROL_OBJECT_ID, 0, COLOUR_RESOURCE_ID);

    AwaClientSetOperation *operation = AwaClientSetOperation_New(session);
    AwaClientSetOperation_CreateObjectInstance(operation, lightControlObjectInstancePath);
    AwaClientSetOperation_CreateOptionalResource(operation, colorResourcePath);
    AwaClientSetOperation_AddValueAsCString(operation, colorResourcePath, "red");
    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);

    // Subscribe to color change event
    AwaClientChangeSubscription *subscription = AwaClientChangeSubscription_New(colorResourcePath, colorChangeCallback, NULL);
    AwaClientSubscribeOperation *subscribeOperation = AwaClientSubscribeOperation_New(session);
    AwaClientSubscribeOperation_AddChangeSubscription(subscribeOperation, subscription);
    AwaClientSubscribeOperation_Perform(subscribeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSubscribeOperation_Free(&subscribeOperation);

    while (_keepRunning)
    {
        AwaClientSession_Process(session, OPERATION_PERFORM_TIMEOUT);
        AwaClientSession_DispatchCallbacks(session);
        sleep(1);
    }

    // Unsubscribe from color change event
    AwaClientSubscribeOperation *cancelSubscribeOperation = AwaClientSubscribeOperation_New(session);
    AwaClientSubscribeOperation_AddCancelChangeSubscription(cancelSubscribeOperation, subscription);
    AwaClientSubscribeOperation_Perform(cancelSubscribeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSubscribeOperation_Free(&cancelSubscribeOperation);
    AwaClientChangeSubscription_Free(&subscription);

    // Delete LightControl IPSO object instance
    AwaClientDeleteOperation *deleteOperation = AwaClientDeleteOperation_New(session);
    AwaClientDeleteOperation_AddPath(deleteOperation, lightControlObjectInstancePath);
    AwaClientDeleteOperation_Perform(deleteOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientDeleteOperation_Free(&deleteOperation);

    // Disconnect Awa client
    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}