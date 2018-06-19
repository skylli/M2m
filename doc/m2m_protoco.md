
# M2M protocol
Transport protocol use in mechine to mechine.Both endpoint are equal.Most of  reference of Coap and Mqtt.

# format
## routing layer
 
    |-----1---------|---------------|---------------|---------------|
    0 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |Ver| hops		  |-- secret type |--- crc -----------------------|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |------------------ stoken -------------------------------------|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |-- destination id0 --------------------------------------------|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |-- destination id1 --------------------------------------------|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |-- source id0 -------------------------------------------------|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |-- source id1 -------------------------------------------------|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |-- payloadlen -----------------|-- session data ---------------|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

	Ver : version.
	hops : Each hop in the route.
    secret type : Secret type and version. Every message must be clear it's secret version.
	
	
## session

    |-----1---------|---------------|---------------|---------------|
    0 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |Ver| T |  TKL  |--- code ------|--- message id-|--- token -----|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |--- option ----------------------------------------------------|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |1 1 1 1 1 1 1 1| payload length|--- payload -------------------|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    
    - Version (Ver):  2-bit unsigned integer.  Indicates the m2m version
      number.  Implementations of this specification MUST set this field
      to 1 (01 binary).  Other values are reserved for future versions.
      Messages with unknown version numbers MUST be silently ignored.
      
    -Type (T):  2-bit unsigned integer.  Indicates if this message is of`
      type Confirmable (0), Non-confirmable (1), Acknowledgement (2), or
      Reset (3).
      
    Token Length (TKL):  4-bit unsigned integer.  Indicates the length of
      the variable-length Token field.
      
    Code:  8-bit unsigned integer, split into a 3-bit class (most
      significant bits) and a 5-bit detail (least significant bits),
      documented as "c.dd" where "c" is a digit from 0 to 7 for the
      3-bit subfield and "dd" are two digits from 00 to 31 for the 5-bit
      subfield.  The class can indicate a request (0), a success
      response (2), a client error response (4), or a server error
      response (5).  (All other class values are reserved.)  As a
      special case, Code 0.00 indicates an Empty message.  In case of a
      request, the Code field indicates the Request Method; in case of a
      response, a Response Code.  Possible values are maintained in the
      CoAP Code Registries (Section 12.1)。
    
    Message ID:  8-bit in network byte order.  Used to
      detect message duplication and to match messages of type
      Acknowledgement/Reset to messages of type Confirmable/Non-
      confirmable.
    Token: The Token is used to match a response with a request.Every request
    carries a client-generated token that the sever must echo(without modification)
    in any resulting reponse.
    
    Options: Both request and responses may include a list of one or more options.  
## option format 

    0   1   2   3   4   5   6   7
    +---------------+---------------+
    |               |               |
    |  Option Delta | Option Length |   1 byte
    |               |               |
    +---------------+---------------+
    \                               \
    /         Option Delta          /   0-2 bytes
    \          (extended)           \
    +-------------------------------+
    \                               \
    /         Option Length         /   0-2 bytes
    \          (extended)           \
    +-------------------------------+
    \                               \
    /                               /
    \                               \
    /         Option Value          /   0 or more bytes
    \                               \
    /                               /
    \                               \
    +-------------------------------+
    
    CoAP defines a number of options that can be included in a message.
    Each option instance in a message specifies the Option Number of the
    defined CoAP option, the length of the Option Value, and the Option
    Value itself.
    
    Instead of specifying the Option Number directly, the instances MUST
    appear in order of their Option Numbers and a delta encoding is used
    between them: the Option Number for each instance is calculated as
    the sum of its delta and the Option Number of the preceding instance
    in the message.  For the first instance in a message, a preceding
    option instance with Option Number zero is assumed.  Multiple
    instances of the same option can be included by using a delta of
    zero.
    
    The fields in an option are defined as follows:
    
    Option Delta:  4-bit unsigned integer.  A value between 0 and 12
      indicates the Option Delta.  Three values are reserved for special
      constructs:
    
      13:  An 8-bit unsigned integer follows the initial byte and
         indicates the Option Delta minus 13.
      14:  A 16-bit unsigned integer in network byte order follows the
         initial byte and indicates the Option Delta minus 269.
    
      15:  Reserved for the Payload Marker.  If the field is set to this
         value but the entire byte is not the payload marker, this MUST
         be processed as a message format error.
    
    The resulting Option Delta is used as the difference between the
      Option Number of this option and that of the previous option (or
      zero for the first option).  In other words, the Option Number is
      calculated by simply summing the Option Delta values of this and
      all previous options before it.
    
    Option Length:  4-bit unsigned integer.  A value between 0 and 12
      indicates the length of the Option Value, in bytes.  Three values
      are reserved for special constructs:
    
      13:  An 8-bit unsigned integer precedes the Option Value and
         indicates the Option Length minus 13.
    
      14:  A 16-bit unsigned integer in network byte order precedes the
         Option Value and indicates the Option Length minus 269.
    
      15:  Reserved for future use.  If the field is set to this value,
         it MUST be processed as a message format error.
    
    Value:  A sequence of exactly Option Length bytes.  The length and
      format of the Option Value depend on the respective option, which
      MAY define variable-length values.  See Section 3.2 for the
      formats used in this document; options defined in other documents
      MAY make use of other option value formats.
