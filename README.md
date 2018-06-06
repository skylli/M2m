
# Describe
M2M protocol is use in communication between machines. Both endpoint are equal.Most of  reference of Coap and Mqtt.

# Directory structure
```shell
.
├── app
├── bin
├── build
│   ├── CMakeLists.txt
│   └── M2mConfig.h.in
├── config
│   └── m2m.conf
├── doc
│   └── m2m_protoco.md
├── include
│   ├── m2m.h       # m2m sdk and the 3rd party Application need to be included.
│   ├── m2m_api.h   # api that provide to thrid party applications.
│   ├── m2m_type.h  # data format declaration.
│   ├── util.h
│   └── utlist.h
├── lib
├── package
├── README.md
└── src
    ├── crypt
    ├── network
    └── util

```
# target 

    - Build m2m library for arduino/embedded device/server/ios or android application.
    - Build simple sample application for this library.

    



