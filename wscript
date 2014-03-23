## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('sm4ns3', ['core', 'mobility' , 'internet', 'wifi', 'network', 'wave'])
    module.includes =['.' , 
        'model', 
        'model/network' ,
        'model/message',
        'model/message/base',
        'model/message/roadrunner',
        'roadrunner_testing',
    ]
    
    module.source = [
        'model/profile_builder.cc',
        'model/smb_broker.cc',
        'model/network/registration.cc',
        'model/network/smb_connection.cc',
        'model/smb_agent.cc',
        'model/smb_base_wifi.cc',
        'model/smb_base_mobility.cc',
        'model/smb_base_ip.cc',
        'model/network/smb_session.cc',
        'model/message/handlers.cc',
        'model/message/serialize.cc',
        'model/testing/rr_baseline.cc',
    ]

    #TODO: The first two should go.
    module.use.append("BOOST_SYSTEM")
    module.use.append("BOOST_THREAD")
    module.use.append("JSONCPP")

    headers = bld(features='ns3header')
    headers.module = 'sm4ns3'
    headers.source = [
        'model/profile_builder.h',
        'model/smb_broker.h',
        'model/network/registration.h',
        'model/network/smb_connection.h',
        'model/smb_agent.h',
        'model/smb_base_wifi.h',
        'model/smb_base_mobility.h',
        'model/smb_base_ip.h',
        'model/network/smb_session.h',
        'model/message/message_base.h',
        'model/message/messages.h',
        'model/message/handler_base.h',
        'model/message/handlers.h',
        'model/thread_safe_queue.h',
        'model/message/serialize.h',
        'model/testing/rr_baseline.h',
    ]

    #Build examples directory.
    if (bld.env['ENABLE_EXAMPLES']):
        bld.recurse('examples')

