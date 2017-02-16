/*
 *
 *  EW17 Bulb Controller
 *
 *  Copyright (C) 2017  Imagination Technologies Limited
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <string.h>
#include <stdio.h>
#include "awa.h"
#include "colors.h"

static AwaClientSession *session;
static AwaClientChangeSubscription *subscription;
static ColorChangedCB color_changed_cb;
static char lightControlObjectInstancePath[40];
static char colorResourcePath[40];
extern Color *colortmp;
static void awa_color_changed_cb(const AwaChangeSet *changeSet, void *context)
{
    const char *value;
    AwaChangeSet_GetValueAsCStringPointer(changeSet, "/3311/0/5706", &value);
    int i = 0;
    bool validColour = false;

    for (i = 0; i < 7; i++)
    {
        if (strcmp(colors[i].colorName, value) == 0)
        {
            validColour = true;
            break;
        }
    }
    if (validColour)
    {
        g_message("Value of resource /3311/0/5706 changed to: %s", value);

        color_changed_cb(value);
    }
    else
    {
        g_warning("Value of resource /3311/0/5706 changed to: %s but it is invalid color.", value);
    }
}

AwaClientSession* awa_setup(ColorChangedCB colorchangedCB, GError **error)
{
    color_changed_cb = colorchangedCB;
    session = AwaClientSession_New();

    AwaError result = AwaClientSession_Connect(session);
    if (result != AwaError_Success) {
        g_set_error(error, 1 , result, "Unable to establish awa session");
        return NULL;
    }

    // Define Lightcontrol IPSO object if not exists
    if (AwaClientSession_IsObjectDefined(session, 3311) == false)
    {
        AwaObjectDefinition *objectDefinition = AwaObjectDefinition_New(
        LIGHT_CONTROL_OBJECT_ID, "LightControl", 0, 1);
        AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition,
        COLOUR_RESOURCE_ID, "Color", false, AwaResourceOperations_ReadWrite,
        NULL);
        AwaClientDefineOperation *operationDefine = AwaClientDefineOperation_New(session);
        AwaClientDefineOperation_Add(operationDefine, objectDefinition);
        AwaClientDefineOperation_Perform(operationDefine,
        OPERATION_PERFORM_TIMEOUT);
        AwaClientDefineOperation_Free(&operationDefine);
    }

    // Create LightControl IPSO object instance

    sprintf(&lightControlObjectInstancePath[0], "/%d/%d", LIGHT_CONTROL_OBJECT_ID, 0);
    sprintf(&colorResourcePath[0], "/%d/%d/%d", LIGHT_CONTROL_OBJECT_ID, 0, COLOUR_RESOURCE_ID);

    AwaClientSetOperation *operation = AwaClientSetOperation_New(session);
    AwaClientSetOperation_CreateObjectInstance(operation, lightControlObjectInstancePath);
    AwaClientSetOperation_CreateOptionalResource(operation, colorResourcePath);
    AwaClientSetOperation_AddValueAsCString(operation, colorResourcePath, "red");
    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);

    // Subscribe to color change event
    subscription = AwaClientChangeSubscription_New(colorResourcePath, awa_color_changed_cb, NULL);
    AwaClientSubscribeOperation *subscribeOperation = AwaClientSubscribeOperation_New(session);
    AwaClientSubscribeOperation_AddChangeSubscription(subscribeOperation, subscription);
    AwaClientSubscribeOperation_Perform(subscribeOperation,
    OPERATION_PERFORM_TIMEOUT);
    AwaClientSubscribeOperation_Free(&subscribeOperation);

    return session;

}

void awa_cleanup()
{
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
}

gboolean awa_process(gpointer user_data)
{
    AwaClientSession *session = (AwaClientSession*) user_data;
    AwaClientSession_Process(session, OPERATION_PERFORM_TIMEOUT);
    AwaClientSession_DispatchCallbacks(session);
    return TRUE;
}

void awa_get_current_color(char **color) {
    AwaClientGetOperation *operation = AwaClientGetOperation_New(session);
    AwaClientGetOperation_AddPath(operation, colorResourcePath);
    AwaClientGetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientGetResponse *response = AwaClientGetOperation_GetResponse(operation);
    AwaClientGetResponse_GetValueAsCStringPointer(response, colorResourcePath, color);
}

