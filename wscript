## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('sm4ns3', ['core', 'mobility' , 'internet', 'wifi', 'network'])
    module.includes =['.' , 
        'model', 
        'model/network' ,
        'model/message',
        'model/message/base',
        'model/message/roadrunner',
        'roadrunner_testing',
    ]
    
    module.source = [
        'model/smb_broker.cc',
        'model/message/roadrunner/smb_registration.cc',
        'model/network/smb_connection.cc',
        'model/smb_agent.cc',
        'model/smb_base_wifi.cc',
        'model/smb_base_mobility.cc',
        'model/smb_base_ip.cc',
        'model/smb_configurator.cc',
        'model/message/base/smb_agents_info.cc',
        'model/message/base/smb_readytoreceive_message.cc',
        'model/message/base/smb_all_location.cc',
        'model/message/base/smb_time_info.cc',
        'model/message/base/smb_message_factory_base.cc',
        'model/message/roadrunner/smb_rr_casts.cc',
        'roadrunner_testing/smb_roadrunner_baseline.cc',
    ]

    #TODO: The first two should go.
    module.use.append("BOOST_SYSTEM")
    module.use.append("BOOST_THREAD")
    module.use.append("JSONCPP")

    headers = bld(features='ns3header')
    headers.module = 'sm4ns3'
    headers.source = [
        'model/smb_broker.h',
        'model/message/roadrunner/smb_registration.h',
        'model/network/smb_connection.h',
        'model/smb_agent.h',
        'model/smb_base_wifi.h',
        'model/smb_base_mobility.h',
        'model/smb_base_ip.h',
        'model/smb_configurator.h',
        'model/message/base/smb_agents_info.h',
        'model/message/base/smb_readytoreceive_message.h',
        'model/message/base/smb_all_location.h',
        'model/message/base/smb_time_info.h',
        'model/message/roadrunner/smb_rr_casts.h',
        'model/network/smb_session.h',
        'model/message/base/smb_message_base.h',
        'model/message/base/smb_message_handler_base.h',
        'model/message/base/smb_message_queue.h',
        'model/message/base/smb_message_factory_base.h',
        'model/smb_serializer.h',
        'roadrunner_testing/smb_roadrunner_baseline.h',
    ]

    #Build examples directory.
    if (bld.env['ENABLE_EXAMPLES']):
        bld.recurse('examples')

