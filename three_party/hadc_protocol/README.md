# description
Hardware abstraction and control .
## protocol
协议结构   协议版本 + 命令 + payload 长度 + payload
```shell
    version(u8) + type(u8) + len(u16) + payload
```
## version 
 
version | description
---|---
0x01 | wifi-mode
... | ble
... | zwave

## type
type | description
---|---
0x01 | uart
0x02 | gpio
0x03 | input