/* RecursiveType_publisher.cxx

A publication of data of type Tree

This file is derived from code automatically generated by the rtiddsgen
command:

rtiddsgen -language C++03 -example <arch> RecursiveType.idl

Example publication of type Tree automatically generated by
'rtiddsgen'. To test them follow these steps:

(1) Compile this file and the example subscription.

(2) Start the subscription on the same domain used for RTI Data Distribution
Service with the command
objs/<arch>/RecursiveType_subscriber <domain_id> <sample_count>

(3) Start the publication on the same domain used for RTI Data Distribution
Service with the command
objs/<arch>/RecursiveType_publisher <domain_id> <sample_count>

(4) [Optional] Specify the list of discovery initial peers and
multicast receive addresses via an environment variable or a file
(in the current working directory) called NDDS_DISCOVERY_PEERS.

You can run any number of publishers and subscribers programs, and can
add and remove them dynamically from the domain.

Example:

To run the example application on domain <domain_id>:

On Unix:

objs/<arch>/RecursiveType_publisher <domain_id>
objs/<arch>/RecursiveType_subscriber <domain_id>

On Windows:

objs\<arch>\RecursiveType_publisher <domain_id>
objs\<arch>\RecursiveType_subscriber <domain_id>
*/

#include <iostream>

#include <dds/pub/ddspub.hpp>
#include <rti/util/util.hpp>  // for sleep()

#include "RecursiveType.hpp"

void fill_tree(Tree &tree, int childcount, int depth, int value)
{
    tree.node().data(value);

    if (depth == 0) {
        return;
    }

    if (!tree.children().is_set()) {
        // Initialize Tree with empty vector of children. This
        // call makes a copy of the (empty) vector
        dds::core::vector<Tree> children;
        tree.children(children);
    }

    // Resize the list of children to the desired length
    tree.children().get().resize(childcount);

    for (int i = 0; i < childcount; ++i) {
        fill_tree(tree.children().get()[i], childcount, depth - 1, value);
    }
}

void publisher_main(int domain_id, int sample_count)
{
    // Create a DomainParticipant with default Qos
    dds::domain::DomainParticipant participant(domain_id);

    // Create a Topic -- and automatically register the type
    dds::topic::Topic<Tree> topic(participant, "Example Tree");

    // Create a DataWriter with default Qos (Publisher created in-line)
    dds::pub::DataWriter<Tree> writer(dds::pub::Publisher(participant), topic);

    Tree sample;
    for (int count = 0; count < sample_count || sample_count == 0; count++) {
        // Modify the data to be written here
        fill_tree(sample, 1, count, count);
        std::cout << "Writing Tree, count " << count << std::endl;
        writer.write(sample);

        rti::util::sleep(dds::core::Duration(4));
    }
}


int main(int argc, char *argv[])
{
    int domain_id = 0;
    int sample_count = 0;  // infinite loop

    if (argc >= 2) {
        domain_id = atoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }

    // To turn on additional logging, include <rti/config/Logger.hpp> and
    // uncomment the following line:
    // rti::config::Logger::instance().verbosity(rti::config::Verbosity::STATUS_ALL);

    try {
        publisher_main(domain_id, sample_count);
    } catch (const std::exception &ex) {
        // This will catch DDS exceptions
        std::cerr << "Exception in publisher_main(): " << ex.what()
                  << std::endl;
        return -1;
    }

    // RTI Connext provides a finalize_participant_factory() method
    // if you want to release memory used by the participant factory singleton.
    // Uncomment the following line to release the singleton:
    //
    // dds::domain::DomainParticipant::finalize_participant_factory();

    return 0;
}
