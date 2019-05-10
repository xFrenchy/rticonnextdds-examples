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

/* listenersPublisher.java

   A publication of data of type listeners

   This file is derived from code automatically generated by the rtiddsgen 
   command:

   rtiddsgen -language java -example <arch> .idl

   Example publication of type listeners automatically generated by 
   'rtiddsgen' To test them follow these steps:

   (1) Compile this file and the example subscription.

   (2) Start the subscription with the command
       java listenersSubscriber <domain_id> <sample_count>
       
   (3) Start the publication with the command
       java listenersPublisher <domain_id> <sample_count>

   (4) [Optional] Specify the list of discovery initial peers and 
       multicast receive addresses via an environment variable or a file 
       (in the current working directory) called NDDS_DISCOVERY_PEERS.  
       
   You can run any number of publishers and subscribers programs, and can 
   add and remove them dynamically from the domain.
              
   Example:
        
       To run the example application on domain <domain_id>:
            
       Ensure that $(NDDSHOME)/lib/<arch> is on the dynamic library path for
       Java.                       
       
        On Unix: 
             add $(NDDSHOME)/lib/<arch> to the 'LD_LIBRARY_PATH' environment
             variable
                                         
        On Windows:
             add %NDDSHOME%\lib\<arch> to the 'Path' environment variable
                        

       Run the Java applications:
       
        java -Djava.ext.dirs=$NDDSHOME/class listenersPublisher <domain_id>

        java -Djava.ext.dirs=$NDDSHOME/class listenersSubscriber <domain_id>        

       
       
modification history
------------ -------         
*/

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Arrays;

import com.rti.dds.domain.*;
import com.rti.dds.infrastructure.*;
import com.rti.dds.publication.*;
import com.rti.dds.topic.*;
import com.rti.ndds.config.*;

// ===========================================================================

public class listenersPublisher {
    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------
    
    public static void main(String[] args) {
        // --- Get domain ID --- //
        int domainId = 0;
        if (args.length >= 1) {
            domainId = Integer.valueOf(args[0]).intValue();
        }

        // -- Get max loop count; 0 means infinite loop --- //
        int sampleCount = 0;
        if (args.length >= 2) {
            sampleCount = Integer.valueOf(args[1]).intValue();
        }
        
        /* Uncomment this to turn on additional logging
        Logger.get_instance().set_verbosity_by_category(
            LogCategory.NDDS_CONFIG_LOG_CATEGORY_API,
            LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
        */
        
        // --- Run --- //
        publisherMain(domainId, sampleCount);
    }
    
    
    
    // -----------------------------------------------------------------------
    // Private Methods
    // -----------------------------------------------------------------------
    
    // --- Constructors: -----------------------------------------------------
    
    private listenersPublisher() {
        super();
    }
    
    
    // -----------------------------------------------------------------------
    
    private static void publisherMain(int domainId, int sampleCount) {

        DomainParticipant participant = null;
        Publisher publisher = null;
        Topic topic = null;
        listenersDataWriter writer = null;

        try {
            // --- Create participant --- //
    
            /* To customize participant QoS, use
               the configuration file
               USER_QOS_PROFILES.xml */
    
            participant = DomainParticipantFactory.TheParticipantFactory.
                create_participant(
                    domainId, DomainParticipantFactory.PARTICIPANT_QOS_DEFAULT,
                    null /* listener */, StatusKind.STATUS_MASK_NONE);
            if (participant == null) {
                System.err.println("create_participant error\n");
                return;
            }        
                    
            // --- Create publisher --- //
    
            /* To customize publisher QoS, use
               the configuration file USER_QOS_PROFILES.xml */
    
            publisher = participant.create_publisher(
                DomainParticipant.PUBLISHER_QOS_DEFAULT, null /* listener */,
                StatusKind.STATUS_MASK_NONE);
            if (publisher == null) {
                System.err.println("create_publisher error\n");
                return;
            }                   

	    /* Create ande Delete Inconsistent Topic 
	     * ---------------------------------------------------------------
	     * Here we create an inconsistent topic to trigger the subscriber 
	     * application's callback. 
	     * The inconsistent topic is created with the topic name used in 
	     * the Subscriber application, but with a different data type -- 
	     * the msg data type defined in partitions.idl.
	     * Once it is created, we sleep to ensure the applications discover
	     * each other and delete the Data Writer and Topic.
	     */
    
	    /* First we register the msg type -- we name it
	     * inconsistent_topic_type_name to avoid confusion. 
	     */
	    System.out.println("Creating Inconsistent Topic...  ");

	    // Register a different type than subscriber is expecting
	    String inconsistentTypeName = msgTypeSupport.get_type_name();
	    msgTypeSupport.register_type(participant, inconsistentTypeName);
	    
	    Topic inconsistentTopic = 
		participant.create_topic("Example listeners",
					 inconsistentTypeName, 
					 DomainParticipant.TOPIC_QOS_DEFAULT,
					 null /* listener */, 
					 StatusKind.STATUS_MASK_NONE);

	    /* We have to associate a writer to the topic, as Topic information is not
	     * actually propagated until the creation of an associated writer.
	     */
	    msgDataWriter inconsistentWriter = 
		(msgDataWriter)publisher.create_datawriter(inconsistentTopic, 
							   Publisher.DATAWRITER_QOS_DEFAULT,
							   null /* listener */, 
							   StatusKind.STATUS_MASK_NONE);

	    
	    // Sleep to ensure the apps have had time to discover each other
	    long sleepPeriodMillis = 2000;
	    try {
		Thread.sleep(sleepPeriodMillis);
	    } catch (InterruptedException ix) {
		System.err.println("INTERRUPTED");
	    }
	    
	    
	    publisher.delete_datawriter(inconsistentWriter);
	    participant.delete_topic(inconsistentTopic);

	    System.out.println("... Deleted Inconsistent Topic\n");

            // --- Create topic --- //

            /* Register type before creating topic */
            String typeName = listenersTypeSupport.get_type_name();
            listenersTypeSupport.register_type(participant, typeName);
    
            /* To customize topic QoS, use
               the configuration file USER_QOS_PROFILES.xml */
    
            topic = participant.create_topic(
                "Example listeners",
                typeName, DomainParticipant.TOPIC_QOS_DEFAULT,
                null /* listener */, StatusKind.STATUS_MASK_NONE);
            if (topic == null) {
                System.err.println("create_topic error\n");
                return;
            }           
                
            // --- Create writer --- //
    
	    /* We will use the Data Writer Listener defined above to print
	     * a message when some of events are triggered in the DataWriter. 
	     * To do that, first we have to pass the writer_listener and then
	     * we have to enable all status in the status mask.
	     */
	    DataWriterListener writerListener = new DataWriterListener();
	    writer = (listenersDataWriter)
                publisher.create_datawriter(topic, 
					    Publisher.DATAWRITER_QOS_DEFAULT,
					    writerListener /* listener */, 
					    StatusKind.STATUS_MASK_ALL /* get all statuses */);
            if (writer == null) {
                System.err.println("create_datawriter error\n");
                return;
            }           
                                        
            // --- Write --- //

            /* Create data sample for writing */
            listeners instance = new listeners();

            InstanceHandle_t instance_handle = InstanceHandle_t.HANDLE_NIL;
            /* For a data type that has a key, if the same instance is going to be
               written multiple times, initialize the key here
               and register the keyed instance prior to writing */
            //instance_handle = writer.register_instance(instance);

            final long sendPeriodMillis = 1000; // 1 seconds

            for (int count = 0;
                 (sampleCount == 0) || (count < sampleCount);
                 ++count) {
                System.out.println("Writing listeners, count " + count);

                /* Modify the instance to be written here */
                instance.x = (short)count;
            
                /* Write data */
                writer.write(instance, instance_handle);
                try {
                    Thread.sleep(sendPeriodMillis);
                } catch (InterruptedException ix) {
                    System.err.println("INTERRUPTED");
                    break;
                }
            }

            //writer.unregister_instance(instance, instance_handle);

        } finally {

            // --- Shutdown --- //

            if(participant != null) {
                participant.delete_contained_entities();

                DomainParticipantFactory.TheParticipantFactory.
                    delete_participant(participant);
            }
            /* RTI Connext provides finalize_instance()
               method for people who want to release memory used by the
               participant factory singleton. Uncomment the following block of
               code for clean destruction of the participant factory
               singleton. */
            //DomainParticipantFactory.finalize_instance();
        }
    }
    // -----------------------------------------------------------------------
    // Private Types
    // -----------------------------------------------------------------------
    
    // =======================================================================

    private static class DataWriterListener extends DataWriterAdapter {
	public void on_offered_deadline_missed(
            DataWriter dataWriter,
	    OfferedDeadlineMissedStatus status) 
	{
	    System.out.println("DataWriterListener: on_offered_deadline_missed()\n");
	}
	public void on_liveliness_lost(
            DataWriter writer,
	    LivelinessLostStatus status) 
	{
	    System.out.println("DataWriterListener: on_liveliness_lost()\n");
	}
	
	public void on_offered_incompatible_qos(
            DataWriter writer,
	    OfferedIncompatibleQosStatus status) 
	{
	    System.out.println("DataWriterListener: on_offered_incompatible_qos()\n");
	}
        
	public void on_publication_matched(
            DataWriter writer,
	    PublicationMatchedStatus status) 
	{
	    System.out.println("DataWriterListener: on_publication_matched()\n");
	    if (status.current_count_change < 0) {
		System.out.println("lost a subscription\n");
	    } 
	    else {
		System.out.println("found a subscription\n");
	    }
	}
    
	public void on_reliable_writer_cache_changed(
            DataWriter writer, 
	    ReliableWriterCacheChangedStatus status) 
	{
	    System.out.println("DataWriterListener: on_reliable_writer_cache_changed()\n");
	}

	public void on_reliable_reader_activity_changed(
            DataWriter writer,
	    ReliableReaderActivityChangedStatus status)
	{
	    System.out.println("DataWriterListener: on_reliable_reader_activity_changed()\n");
	}
    }
}

        