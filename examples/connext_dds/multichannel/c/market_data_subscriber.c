/*******************************************************************************
 (c) 2005-2014 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 RTI grants Licensee a license to use, modify, compile, and create derivative
 works of the Software.  Licensee has the right to distribute object form only
 for use with RTI products.  The Software is provided "as is", with no warranty
 of any type, including any warranty for fitness for any purpose. RTI is under
 no obligation to maintain or support the Software.  RTI shall not be liable for
 any incidental or consequential damages arising out of the use or inability to
 use the software.
 ******************************************************************************/
/* market_data_subscriber.c

 A subscription example

 This file is derived from code automatically generated by the rtiddsgen
 command:

 rtiddsgen -language C -example <arch> market_data.idl

 Example subscription of type market_data automatically generated by
 'rtiddsgen'. To test them, follow these steps:

 (1) Compile this file and the example publication.

 (2) Start the subscription with the command
 objs/<arch>/market_data_subscriber <domain_id> <sample_count>

 (3) Start the publication with the command
 objs/<arch>/market_data_publisher <domain_id> <sample_count>

 (4) [Optional] Specify the list of discovery initial peers and
 multicast receive addresses via an environment variable or a file
 (in the current working directory) called NDDS_DISCOVERY_PEERS.

 You can run any number of publishers and subscribers programs, and can
 add and remove them dynamically from the domain.


 Example:

 To run the example application on domain <domain_id>:

 On UNIX systems:

 objs/<arch>/market_data_publisher <domain_id>
 objs/<arch>/market_data_subscriber <domain_id>

 On Windows systems:

 objs\<arch>\market_data_publisher <domain_id>
 objs\<arch>\market_data_subscriber <domain_id>


 modification history
 ------------ -------
 */

#include "market_data.h"
#include "market_dataSupport.h"
#include "ndds/ndds_c.h"
#include <stdio.h>
#include <stdlib.h>

void market_dataListener_on_requested_deadline_missed(
        void *listener_data,
        DDS_DataReader *reader,
        const struct DDS_RequestedDeadlineMissedStatus *status)
{
}

void market_dataListener_on_requested_incompatible_qos(
        void *listener_data,
        DDS_DataReader *reader,
        const struct DDS_RequestedIncompatibleQosStatus *status)
{
}

void market_dataListener_on_sample_rejected(
        void *listener_data,
        DDS_DataReader *reader,
        const struct DDS_SampleRejectedStatus *status)
{
}

void market_dataListener_on_liveliness_changed(
        void *listener_data,
        DDS_DataReader *reader,
        const struct DDS_LivelinessChangedStatus *status)
{
}

void market_dataListener_on_sample_lost(
        void *listener_data,
        DDS_DataReader *reader,
        const struct DDS_SampleLostStatus *status)
{
}

void market_dataListener_on_subscription_matched(
        void *listener_data,
        DDS_DataReader *reader,
        const struct DDS_SubscriptionMatchedStatus *status)
{
}

void market_dataListener_on_data_available(
        void *listener_data,
        DDS_DataReader *reader)
{
    market_dataDataReader *market_data_reader = NULL;
    struct market_dataSeq data_seq = DDS_SEQUENCE_INITIALIZER;
    struct DDS_SampleInfoSeq info_seq = DDS_SEQUENCE_INITIALIZER;
    DDS_ReturnCode_t retcode;
    int i;

    market_data_reader = market_dataDataReader_narrow(reader);
    if (market_data_reader == NULL) {
        printf("DataReader narrow error\n");
        return;
    }

    retcode = market_dataDataReader_take(
            market_data_reader,
            &data_seq,
            &info_seq,
            DDS_LENGTH_UNLIMITED,
            DDS_ANY_SAMPLE_STATE,
            DDS_ANY_VIEW_STATE,
            DDS_ANY_INSTANCE_STATE);
    if (retcode == DDS_RETCODE_NO_DATA) {
        return;
    } else if (retcode != DDS_RETCODE_OK) {
        printf("take error %d\n", retcode);
        return;
    }

    for (i = 0; i < market_dataSeq_get_length(&data_seq); ++i) {
        if (DDS_SampleInfoSeq_get_reference(&info_seq, i)->valid_data) {
            market_dataTypeSupport_print_data(
                    market_dataSeq_get_reference(&data_seq, i));
        }
    }

    retcode = market_dataDataReader_return_loan(
            market_data_reader,
            &data_seq,
            &info_seq);
    if (retcode != DDS_RETCODE_OK) {
        printf("return loan error %d\n", retcode);
    }
}

/* Delete all entities */
static int subscriber_shutdown(DDS_DomainParticipant *participant)
{
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (participant != NULL) {
        retcode = DDS_DomainParticipant_delete_contained_entities(participant);
        if (retcode != DDS_RETCODE_OK) {
            printf("delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDS_DomainParticipantFactory_delete_participant(
                DDS_TheParticipantFactory,
                participant);
        if (retcode != DDS_RETCODE_OK) {
            printf("delete_participant error %d\n", retcode);
            status = -1;
        }
    }

    /* RTI Connext provides the finalize_instance() method on
     domain participant factory for users who want to release memory used
     by the participant factory. Uncomment the following block of code for
     clean destruction of the singleton. */
    /*
     retcode = DDS_DomainParticipantFactory_finalize_instance();
     if (retcode != DDS_RETCODE_OK) {
     printf("finalize_instance error %d\n", retcode);
     status = -1;
     }
     */

    return status;
}

static int subscriber_main(int domainId, int sample_count)
{
    DDS_DomainParticipant *participant = NULL;
    DDS_Subscriber *subscriber = NULL;
    DDS_Topic *topic = NULL;
    struct DDS_DataReaderListener reader_listener =
            DDS_DataReaderListener_INITIALIZER;
    DDS_DataReader *reader = NULL;
    DDS_ReturnCode_t retcode;
    const char *type_name = NULL;
    int count = 0;
    struct DDS_Duration_t poll_period = { 4, 0 };
    DDS_ContentFilteredTopic *filtered_topic = NULL;
    struct DDS_StringSeq expression_parameters;

    /* To customize participant QoS, use
     the configuration file USER_QOS_PROFILES.xml */
    participant = DDS_DomainParticipantFactory_create_participant(
            DDS_TheParticipantFactory,
            domainId,
            &DDS_PARTICIPANT_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        printf("create_participant error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /* To customize subscriber QoS, use
     the configuration file USER_QOS_PROFILES.xml */
    subscriber = DDS_DomainParticipant_create_subscriber(
            participant,
            &DDS_SUBSCRIBER_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (subscriber == NULL) {
        printf("create_subscriber error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /* Register the type before creating the topic */
    type_name = market_dataTypeSupport_get_type_name();
    retcode = market_dataTypeSupport_register_type(participant, type_name);
    if (retcode != DDS_RETCODE_OK) {
        printf("register_type error %d\n", retcode);
        subscriber_shutdown(participant);
        return -1;
    }

    /* To customize topic QoS, use
     the configuration file USER_QOS_PROFILES.xml */
    topic = DDS_DomainParticipant_create_topic(
            participant,
            "Example market_data",
            type_name,
            &DDS_TOPIC_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        printf("create_topic error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /* Start changes for MultiChannel */

    filtered_topic =
            DDS_DomainParticipant_create_contentfilteredtopic_with_filter(
                    participant,
                    "Example market_data",
                    topic,
                    "Symbol MATCH 'A'",
                    &expression_parameters,
                    DDS_STRINGMATCHFILTER_NAME);
    if (filtered_topic == NULL) {
        printf("create_contentfilteredtopic error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /* Set up a data reader listener */
    reader_listener.on_requested_deadline_missed =
            market_dataListener_on_requested_deadline_missed;
    reader_listener.on_requested_incompatible_qos =
            market_dataListener_on_requested_incompatible_qos;
    reader_listener.on_sample_rejected = market_dataListener_on_sample_rejected;
    reader_listener.on_liveliness_changed =
            market_dataListener_on_liveliness_changed;
    reader_listener.on_sample_lost = market_dataListener_on_sample_lost;
    reader_listener.on_subscription_matched =
            market_dataListener_on_subscription_matched;
    reader_listener.on_data_available = market_dataListener_on_data_available;

    /* To customize data reader QoS, use
     the configuration file USER_QOS_PROFILES.xml */
    reader = DDS_Subscriber_create_datareader(
            subscriber,
            DDS_Topic_as_topicdescription(filtered_topic),
            &DDS_DATAREADER_QOS_DEFAULT,
            &reader_listener,
            DDS_STATUS_MASK_ALL);
    if (reader == NULL) {
        printf("create_datareader error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    printf("filter is Symbol MATCH 'A'\n");

    /* Main loop */
    for (count = 0; (sample_count == 0) || (count < sample_count); ++count) {
        /* printf("market_data subscriber sleeping for %d sec...\n",
         poll_period.sec);
         */

        if (count == 3) {
            retcode = DDS_ContentFilteredTopic_append_to_expression_parameter(
                    filtered_topic,
                    0,
                    "D");
            if (retcode != DDS_RETCODE_OK) {
                printf("append_to_expression_parameter %d\n", retcode);
                subscriber_shutdown(participant);
                return -1;
            }
            printf("changed filter to Symbol MATCH 'AD'\n");
        }

        if (count == 6) {
            retcode = DDS_ContentFilteredTopic_remove_from_expression_parameter(
                    filtered_topic,
                    0,
                    "A");
            if (retcode != DDS_RETCODE_OK) {
                printf("append_to_expression_parameter %d\n", retcode);
                subscriber_shutdown(participant);
                return -1;
            }
            printf("changed filter to Symbol MATCH 'D'\n");
        }
        NDDS_Utility_sleep(&poll_period);
    }

    /* End changes for MultiChannel */

    /* Cleanup and delete all entities */
    return subscriber_shutdown(participant);
}

#if defined(RTI_WINCE)
int wmain(int argc, wchar_t **argv)
{
    int domainId = 0;
    int sample_count = 0; /* infinite loop */

    if (argc >= 2) {
        domainId = _wtoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = _wtoi(argv[2]);
    }

    /* Uncomment this to turn on additional logging
     NDDS_Config_Logger_set_verbosity_by_category(
     NDDS_Config_Logger_get_instance(),
     NDDS_CONFIG_LOG_CATEGORY_API,
     NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
     */

    return subscriber_main(domainId, sample_count);
}
#elif !(defined(RTI_VXWORKS) && !defined(__RTP__)) && !defined(RTI_PSOS)
int main(int argc, char *argv[])
{
    int domainId = 0;
    int sample_count = 0; /* infinite loop */

    if (argc >= 2) {
        domainId = atoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }

    /* Uncomment this to turn on additional logging
     NDDS_Config_Logger_set_verbosity_by_category(
     NDDS_Config_Logger_get_instance(),
     NDDS_CONFIG_LOG_CATEGORY_API,
     NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
     */

    return subscriber_main(domainId, sample_count);
}
#endif

#ifdef RTI_VX653
const unsigned char *__ctype = NULL;

void usrAppInit()
{
    #ifdef USER_APPL_INIT
    USER_APPL_INIT; /* for backwards compatibility */
    #endif

    /* add application specific code here */
    taskSpawn(
            "sub",
            RTI_OSAPI_THREAD_PRIORITY_NORMAL,
            0x8,
            0x150000,
            (FUNCPTR) subscriber_main,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0);
}
#endif
