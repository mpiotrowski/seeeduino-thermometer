This document describes the protocol between the thermomether and the server responsible for processing the messages.

# Limitations

Given that thermomether will run on Seeeduino Nano, it has a limited amount of RAM (2kb) so we need to be very conservative how exactly the memory is going to be handled.

# Establishing the connection

1. Check EEPROM @ 0 to check if the connection has been established before
    1. If byte @ 0 != 0 then continue to step #3
2. 

# Message format

The protocol will be protected by AES 128 (RAM limitations) so our message size will only by 16 bytes, following this format:
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Random Number | Message Type | Reserved | 0x5D | Reserved | 0x25 | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Checksum |

Checksum is defined as `Sum( bytes 0 - 7 ) XOR Sum( Bytes 8 - 14 )`

Defined Message Type:
1. Health Check `0x01` - all reserved bytes are set to 0x00
2. Ask Time Sync `0x02` - all reserved bytes are set to 0x00
3. Send Time Sync `0x04' - bytes 7-10 receive a timestamp
4.  
