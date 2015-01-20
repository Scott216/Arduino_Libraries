/*
  Copyright (c) 2012 Oracle and/or its affiliates. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

  mysql.cpp - Library for communicating with a MySQL Server over Ethernet.
            
  This code module implements the Connector class for connecting to and
  issuing queries against a MySQL database. You can issue any command
  using SQL statements for inserting or retrieving data.
  
  Created by Dr. Charles A. Bell, April 2012.
  
  Motivation: To demonstrate the versatility of the MySQL client protocol
  for adaptation to embedded hardware and to bring MySQL to the Ardiuno
  community.

*/
#include "Arduino.h"
#include "mysql.h"
#include <sha1.h>

#define MAX_CONNECT_ATTEMPTS 3
#define MAX_TIMEOUT          10

// Begin public methods

/**
 * mysql_connect - Connect to a MySQL server.
 *
 * This method is used to connect to a MySQL server. It will attempt to
 * connect to the server as a client retrying up to MAX_CONNECT_ATTEMPTS.
 * This permits the possibility of longer than normal network lag times
 * for wireless networks. You can adjust MAX_CONNECT_ATTEMPTS to suit
 * your environment.
 *
 * server[in]      IP address of the server as IPAddress type
 * port[in]        port number of the server
 * user[in]        user name
 * password[in]    (optional) user password
 *
 * Returns boolean - True = connection succeeded
*/
boolean Connector::mysql_connect(IPAddress server, int port,
                                 char *user, char *password) 
{
  int connected = 0;
  int i = -1;
  ok_packet *packet;
  
  // Retry up to MAX_CONNECT_ATTEMPTS times 1 second apart.
  do {
    delay(1000);
    connected = client.connect(server, port);
    i++;
  } while (i < MAX_CONNECT_ATTEMPTS && !connected);

  if (connected) {
    read_packet();
    parse_handshake_packet();
    send_authentication_packet(user, password);
    read_packet();
    packet = (ok_packet *)malloc(sizeof(ok_packet));
    if (parse_ok_packet(packet) != 0) {
      free(packet);
      parse_error_packet();
      return false;
    }
    free(packet);
    Serial.print("Connected to server version ");
    Serial.print(server_version);
    Serial.println(".");
    free(server_version); // don't need it anymore
    return true;
  }
  return false;
}


/**
 * cmd_query - Execute a SQL statement
 *
 * This method executes the query specified as a character array that is
 * located in data memory. It copies the query to the local buffer then
 * calls the run_query() method to execute the query.
 * 
 * If a result set is available after the query executes, the field
 * packets and rows can be read separately using the get_field() and
 * get_row() methods.
 *
 * query[in]       SQL statement (using normal memory access)
 *
 * Returns boolean - True = a result set is available for reading
*/
boolean Connector::cmd_query(const char *query)
{
  int query_len = (int)strlen(query);
  
  if (buffer != NULL)
    free(buffer);
    
  buffer = (byte *)malloc(query_len+5);

  // Write query to packet
  memcpy(&buffer[5], query, query_len);

  // Send the query
  return run_query(query_len);
}


/**
 * cmd_query_P - Execute a SQL statement
 *
 * This method executes the query specified as a character array that is
 * located in program memory. It copies the query to the local buffer then
 * calls the run_query() method to execute the query.
 * 
 * If a result set is available after the query executes, the field
 * packets and rows can be read separately using the get_field() and
 * get_row() methods.
 *
 * query[in]       SQL statement (using PROGMEM)
 *
 * Returns boolean - True = a result set is available for reading
*/
boolean Connector::cmd_query_P(const char *query)
{
  int query_len = (int)strlen_P(query);
  
  if (buffer != NULL)
    free(buffer);
    
  buffer = (byte *)malloc(query_len+5);

  // Write query to packet
  for (int c = 0; c < query_len; c++)
    buffer[c+5] = pgm_read_byte_near(query+c);

  // Send the query
  return run_query(query_len);
}


/**
 * get_field - Read a field from the server
 *
 * This method reads a field packet from the server. Field packets are
 * defined as:
 * 
 * Bytes                      Name
 * -----                      ----
 * n (Length Coded String)    catalog
 * n (Length Coded String)    db
 * n (Length Coded String)    table
 * n (Length Coded String)    org_table
 * n (Length Coded String)    name
 * n (Length Coded String)    org_name
 * 1                          (filler)
 * 2                          charsetnr
 * 4                          length
 * 1                          type
 * 2                          flags
 * 1                          decimals
 * 2                          (filler), always 0x00
 * n (Length Coded Binary)    default
 * 
 * Note: the sum of all db, column, and field names must be < 255 in length
 * 
*/
int Connector::get_field(field_struct *fs) {
  int len_bytes;
  int len;
  int offset;
  
  // Read field packets until EOF
  read_packet();
  if (buffer[4] != EOF_PACKET) {
    // calculate location of db
    len_bytes = get_lcb_len(4);
    len = read_int(4, len_bytes);
    offset = 4+len_bytes+len;
    fs->db = read_string(&offset);
    // get table
    fs->table = read_string(&offset);
    // calculate location of name
    len_bytes = get_lcb_len(offset);
    len = read_int(offset, len_bytes);
    offset += len_bytes+len;
    fs->name = read_string(&offset);
    return 0;
  }
  return EOF_PACKET;
}


/**
 * get_rows - Read a row from the server and store it in the buffer
 *
 * This reads a single row and stores it in the buffer. If there are
 * no more rows, it returns EOF_PACKET. A row packet is defined as
 * follows.
 *
 * Bytes                   Name
 * -----                   ----
 * n (Length Coded String) (column value)
 * ...
 *
 * Note: each column is store as a length coded string concatenated
 *       as a single stream
 *
 * Returns integer - EOF_PACKET if no more rows, 0 if more rows available
*/
int Connector::get_row() {  
  // Read row packets
  read_packet();
  if (buffer[4] != EOF_PACKET)
    return 0;
  return EOF_PACKET;
}


/**
 * show_results - Show a result set from the server via Serial.print
 *
 * This method reads a result from the server and displays it via the
 * via the Serial.print methods. It can be used in cases where
 * you may want to issue a SELECT or SHOW and see the results on your
 * computer from the Arduino.
 *
 * It is also a good example of how to read a result set from the server.
 *
 * Reading a Result Set
 * --------------------
 * To read a result set from the server, you must first read the result
 * set header, the field packets, then the data rows. Specifically,
 * you must anticipate, read, and parse the following packets.
 *
 * (Result Set Header Packet)  the number of columns
 * (Field Packets)             column descriptors
 * (EOF Packet)                marker: end of Field Packets
 * (Row Data Packets)          row contents
 * (EOF Packet)                marker: end of Data Packets
 *
 * The result header packet is defined as:
 *
 * Bytes                        Name
 * -----                        ----
 * 1-9   (Length-Coded-Binary)  field_count
 * 1-9   (Length-Coded-Binary)  extra
 *
 * See get_field() method for a description of the field packet.
 *
 * See the parse_eof_packet() method for a description of the eof packet.
 *
 * See the get_row() method for a description of the row packet.
 *
 * The process therefore is as follows:
 *   1. read result set header for number of columns
 *   2. read fields until EOF
 *   3. read rows until EOF
 * 
 * Note: this method was provided mainly for documentation purposes. If
 *       you never return result sets or never want to see them in your
 *       serial monitor, you can remove this method to save some bytes.
*/
void Connector::show_results() {
  char *str;
  int offset; 
  int num_fields = buffer[4]; // From result header packet
  int res = 0;
  int rows = 0;
  
  for (int f = 0; f < num_fields; f++) {
    field_struct *field = (field_struct *)malloc(sizeof(field_struct));
    res = get_field(field);
    if (res == EOF_PACKET) {
      Serial.println("Bad mojo. EOF found.");
      return;
    }
    Serial.print(field->name);
    if (f < num_fields-1)
      Serial.print(',');
    free(field->db);
    free(field->table);
    free(field->name);
    free(field);
  }
  Serial.println();      
  read_packet(); // EOF packet
  
  // Read the rows
  do {
    res = get_row();
    if (res != EOF_PACKET) {
      rows++;
      // print column data
      offset = 4;
      for (int f = 0; f < num_fields; f++) {
        str = read_string(&offset);
        Serial.print(str);
        if (f < num_fields-1)
          Serial.print(',');
        free(str);
      }
      Serial.println();
    }
  } while (res != EOF_PACKET);

  // Report how many rows were read
  Serial.print(rows);
  Serial.println(" rows in result.");
}

// Begin private methods

/**
 * run_query - execute a query
 *
 * This method sends the query string to the server and waits for a
 * response. If the result is a result set, it returns true, if it is
 * an error, it processes the error packet and prints the error via
 * Serial.print(). If it is an Ok packet, it parses the packet and
 * returns false.
 *
 * query_len[in]   Number of bytes in the query string
 *
 * Returns boolean - true = result set available,
 *                   false = no result set returned.
*/
boolean Connector::run_query(int query_len)
{
  // TODO: need a 'store_int' method here!
  store_int(&buffer[0], query_len+1, 3);
  // TODO: Abort if query larger than sizeof(buffer);
  buffer[3] = byte(0x00);
  buffer[4] = byte(0x03);  // command packet
  
  // Send the query
  for (int c = 0; c < query_len+5; c++)
    client.write(buffer[c]);

  // Read a response packet and check it for Ok or Error.
  read_packet();
  ok_packet *p = (ok_packet *)malloc(sizeof(ok_packet));
  int res = parse_ok_packet(p);
  free(p);
  if (res == ERROR_PACKET) {
    parse_error_packet();
    return false;
  } else if (!res) {
    return false;
  }
  // Not an Ok packet, so we now have the result set to process.
  return true;
}


/**
 * wait_for_client - Wait until data is available for reading
 *
 * This method is used to permit the connector to respond to servers
 * that have high latency or execute long queries. The timeout is
 * set by MAX_TIMEOUT. Adjust this value to match the performance of
 * your server and network.
 *
 * It is also used to read how many bytes in total are available from the
 * server. Thus, it can be used to know how large a data burst is from
 * the server.
 *
 * Returns integer - Number of bytes available to read.
*/
int Connector::wait_for_client() {
  int num = 0;
  int timeout = 0;
  do {
    delay(500); // adjust for network latency
    num = client.available();
    timeout++;
  } while (num == 0 and timeout < MAX_TIMEOUT);
  return num;
}


/**
 * send_authentication_packet - Send the response to the server's challenge
 *
 * This method builds a response packet used to respond to the server's
 * challenge packet (called the handshake packet). It includes the user
 * name and password scrambled using the SHA1 seed from the handshake
 * packet. It also sets the character set (default is 8 which you can
 * change to meet your needs).
 *
 * Note: you can also set the default database in this packet. See
 *       the code before for a comment on where this happens.
 *
 * The authentication packet is defined as follows.
 *
 * Bytes                        Name
 * -----                        ----
 * 4                            client_flags
 * 4                            max_packet_size
 * 1                            charset_number
 * 23                           (filler) always 0x00...
 * n (Null-Terminated String)   user
 * n (Length Coded Binary)      scramble_buff (1 + x bytes)
 * n (Null-Terminated String)   databasename (optional
 * 
 * user[in]        User name
 * password[in]    password
*/
void Connector::send_authentication_packet(char *user, char *password)
{
  if (buffer != NULL)
    free(buffer);
    
  buffer = (byte *)malloc(256);

  int size_send = 4;
  
  // client flags
  buffer[size_send] = byte(0x85);
  buffer[size_send+1] = byte(0xa6);
  buffer[size_send+2] = byte(0x03);
  buffer[size_send+3] = byte(0x00);
  size_send += 4;
  
  // max_allowed_packet 
  buffer[size_send] = 0; 
  buffer[size_send+1] = 0;
  buffer[size_send+2] = 0;
  buffer[size_send+3] = 1;
  size_send += 4;
  
  // charset - default is 8
  buffer[size_send] = byte(0x08);   
  size_send += 1;
  for(int i = 0; i < 24; i++)
    buffer[size_send+i] = 0x00;
  size_send += 23;

  // user name
  memcpy((char *)&buffer[size_send], user, strlen(user));
  size_send += strlen(user) + 1;
  buffer[size_send-1] = 0x00;

  // password - see scramble password
  byte *scramble = (uint8_t *)malloc(20);
  if (scramble_password(password, scramble)) {
    buffer[size_send] = 0x14;
    size_send += 1;
    for (int i = 0; i < 20; i++)
      buffer[i+size_send] = scramble[i];
    size_send += 20;
    buffer[size_send] = 0x00;
  }
  free(scramble);

  // terminate password response
  buffer[size_send] = 0x00;
  size_send += 1;
  
  // database
  buffer[size_send+1] = 0x00;
  size_send += 1;

  // Write packet size
  int p_size = size_send - 4;
  store_int(&buffer[0], p_size, 3);
  buffer[3] = byte(0x01);

  // Write the packet
  for (int i = 0; i < size_send; i++)
    client.write(buffer[i]);
}


/**
 * scramble_password - Build a SHA1 scramble of the user password
 *
 * This method uses the password hash seed sent from the server to
 * form a SHA1 hash of the password. This is used to send back to
 * the server to complete the challenge and response step in the
 * authentication handshake.
 *
 * password[in]    User's password in clear text
 * pwd_hash[in]    Seed from the server
 *
 * Returns boolean - True = scramble succeeded
*/
boolean Connector::scramble_password(char *password, byte *pwd_hash) {
  byte *digest;
  byte hash1[20];
  byte hash2[20];
  byte hash3[20];
  byte pwd_buffer[40];
  
  if (strlen(password) == 0)
    return false;

  // hash1
  Sha1.init();
  Sha1.print(password);
  digest = Sha1.result();
  memcpy(hash1, digest, 20);

  // hash2
  Sha1.init();
  Sha1.write(hash1, 20);
  digest = Sha1.result();
  memcpy(hash2, digest, 20);

  // hash3 of seed + hash2
  Sha1.init();
  memcpy(pwd_buffer, &seed, 20);
  memcpy(pwd_buffer+20, hash2, 20);
  Sha1.write(pwd_buffer, 40);
  digest = Sha1.result();
  memcpy(hash3, digest, 20);
  
  // XOR for hash4
  for (int i = 0; i < 20; i++)
    pwd_hash[i] = hash1[i] ^ hash3[i];

  return true;
}


/**
 * read_packet - Read a packet from the server and store it in the buffer
 *
 * This method reads the bytes sent by the server as a packet. All packets
 * have a packet header defined as follows.
 *
 * Bytes                 Name
 * -----                 ----
 * 3                     Packet Length
 * 1                     Packet Number
 *
 * Thus, the length of the packet (not including the packet header) can
 * be found by reading the first 4 bytes from the server then reading
 * N bytes for the packet payload.
*/
void Connector::read_packet() {  
  byte local[4];
  
  if (buffer != NULL)
    free(buffer);
  
  // Wait for client (the server) to send data
  wait_for_client();

  // Read packet header
  for (int i = 0; i < 4; i++) 
    local[i] = client.read();
  
  // Get packet length
  packet_len = local[0];
  packet_len += (local[1] << 8);
  packet_len += ((uint32_t)local[2] << 16);
  buffer = (byte *)malloc(packet_len+4);
  
  // read packet number is in buffer[3]
  for (int i = 0; i < 4; i++)
    buffer[i] = local[i];

  for (int i = 4; i < packet_len+4; i++) 
    buffer[i] = client.read();
    
}


/**
 * parse_handshake_packet - Decipher the server's challenge data
 *
 * This method reads the server version string and the seed from the
 * server. The handshake packet is defined as follows.
 *
 *  Bytes                        Name
 *  -----                        ----
 *  1                            protocol_version
 *  n (Null-Terminated String)   server_version
 *  4                            thread_id
 *  8                            scramble_buff
 *  1                            (filler) always 0x00
 *  2                            server_capabilities
 *  1                            server_language
 *  2                            server_status
 *  2                            server capabilities (two upper bytes)
 *  1                            length of the scramble seed 
 * 10                            (filler)  always 0
 *  n                            rest of the plugin provided data
 *                               (at least 12 bytes)
 *  1                            \0 byte, terminating the second part of
 *                                a scramble seed
*/
void Connector::parse_handshake_packet() {
  
  int i = 5;
  do {
    i++;
  } while (buffer[i-1] != 0x00);

  server_version = (char *)malloc(i-5);
  strncpy(server_version, (char *)&buffer[5], i-5);
  
  // Capture the first 8 characters of seed
  i += 4; // Skip thread id
  for (int j = 0; j < 8; j++)
    seed[j] = buffer[i+j];

  // Capture rest of seed
  i += 27; // skip ahead
  for (int j = 0; j < 12; j++)
    seed[j+8] = buffer[i+j];

}

/**
 * parse_eof_packet - Decipher an end of file packet
 *
 * This will read the warnings and flags of an EOF packet defined as
 * follows.
 *
 *  Bytes                 Name
 *  -----                 ----
 *  1                     field_count, always = 0xfe
 *  2                     warning_count
 *  2                     Status Flags
 *  
 * packet[in]      location in the buffer of packet
*/
void Connector::parse_eof_packet(eof_packet *packet) {
  packet->warnings = read_int(5, 2);
  packet->flags = read_int(7, 2);
}


/**
 * parse_error_packet - Display the error returned from the server
 *
 * This method parses an error packet from the server and displays the
 * error code and text via Serial.print. The error packet is defined
 * as follows.
 *
 * Note: the error packet is already stored in the buffer since this
 *       packet is not an expected response. 
 *
 * Bytes                       Name
 * -----                       ----
 * 1                           field_count, always = 0xff
 * 2                           errno
 * 1                           (sqlstate marker), always '#'
 * 5                           sqlstate (5 characters)
 * n                           message
*/
void Connector::parse_error_packet() {
  Serial.print("Error: ");
  Serial.print(read_int(5, 2));
  Serial.print(" = ");
  for (int i = 0; i < packet_len-9; i++)
    Serial.print((char)buffer[i+13]);
  Serial.println(".");
}


/**
 * parse_ok_packet - Decipher an Ok packet from the server.
 *
 * This method attempts to parse an Ok packet. If the packet is not an
 * Ok, packet, it returns the packet type. Otherwise, it attempts to
 * parse the Ok packet defined by the following.
 *
 *  Bytes                       Name
 *  -----                       ----
 *  1   (Length Coded Binary)   field_count, always = 0
 *  1-9 (Length Coded Binary)   affected_rows
 *  1-9 (Length Coded Binary)   insert_id
 *  2                           server_status
 *  2                           warning_count
 *  n   (until end of packet)   message
 *
 * packet[in]      location in the buffer of packet
 *
 * Returns integer - 0 = successful parse, packet type if not an Ok packet
*/
int Connector::parse_ok_packet(ok_packet *packet) {
  int type = buffer[4];
  if (type != OK_PACKET)
    return type;
  
  packet->affected_rows = read_int(5, 0);
  packet->insert_id = read_int(6, 0);
  packet->server_status = read_int(7, 2);
  packet->warning_count = read_int(9, 2);
  memcpy((char *)&packet->message, (char *)&buffer[11], packet_len-7);
  return 0;
}


/**
 * get_lcb_len - Retrieves the length of a length coded binary value
 *
 * This reads the first byte from the offset into the buffer and returns
 * the number of bytes (size) that the integer consumes. It is used in
 * conjunction with read_int() to read length coded binary integers
 * from the buffer.
 *
 * Returns integer - number of bytes integer consumes
*/
int Connector::get_lcb_len(int offset) {
  int read_len = buffer[offset];
  if (read_len > 250) {
    // read type:
    byte type = buffer[offset+1];
    if (type == 0xfc)
      read_len = 2;
    else if (type == 0xfd)
      read_len = 3;
    else if (type == 0xfe)
      read_len = 8;
  }
  return 1; 
}


/**
 * read_string - Retrieve a string from the buffer
 *
 * This reads a string from the buffer. It reads the length of the string
 * as the first byte.
 *
 * offset[in]      offset from start of buffer
 *
 * Returns string - String from the buffer
*/
char *Connector::read_string(int *offset) {
  int len_bytes = get_lcb_len(buffer[*offset]);
  int len = read_int(*offset, len_bytes);
  char *str = (char *)malloc(len+1);
  strncpy(str, (char *)&buffer[*offset+len_bytes], len);
  str[len] = 0x00;
  *offset += len_bytes+len;
  return str;
}


/**
 * read_int - Retrieve an integer from the buffer in size bytes.
 *
 * This reads an integer from the buffer at offset position indicated for
 * the number of bytes specified (size).
 *
 * offset[in]      offset from start of buffer
 * size[in]        number of bytes to use to store the integer
 *
 * Returns integer - integer from the buffer
*/
int Connector::read_int(int offset, int size) {  
  int value = 0;
  int new_size = 0;
  if (size == 0)
     new_size = get_lcb_len(offset);
  if (size == 1)
     return buffer[offset];
  new_size = size;
  int shifter = (new_size - 1) * 8;
  for (int i = new_size; i > 0; i--) {
    value += (byte)(buffer[i-1] << shifter);
    shifter -= 8;
  }
  return value;
}


/**
 * store_int - Store an integer value into a byte array of size bytes.
 *
 * This writes an integer into the buffer at the current position of the
 * buffer. It will transform an integer of size to a length coded binary
 * form where 1-3 bytes are used to store the value (set by size).
 *
 * buff[in]        pointer to location in internal buffer where the
 *                 integer will be stored
 * value[in]       integer value to be stored
 * size[in]        number of bytes to use to store the integer
*/
void Connector::store_int(byte *buff, long value, int size) {
  memset(buff, 0, size);
  if (value < 0xff)
    buff[0] = (byte)value;
  else if (value < 0xffff) {
    buff[0] = (byte)value;
    buff[1] = (byte)(value << 8);
  } else if (value < 0xffffff) {
    buff[0] = (byte)value;
    buff[1] = (byte)(value << 8);
    buff[2] = (byte)(value << 16);
  } else if (value < 0xffffff) {
    buff[0] = (byte)value;
    buff[1] = (byte)(value << 8);
    buff[2] = (byte)(value << 16);
    buff[3] = (byte)(value << 24);
  } 
}


/**
 * print_packet - Print the contents of a packet via Serial.print
 *
 * This method is a diagnostic method. It is best used to decipher a
 * packet from the server (or before being sent to the server). If you
 * are looking for additional program memory space, you can safely
 * delete this method.
*/
void Connector::print_packet() {
  Serial.print("Packet: ");
  Serial.print(buffer[3]);
  Serial.print(" contains ");
  Serial.print(packet_len);
  Serial.println(" bytes.");

  Serial.print("  HEX: ");
  for (int i = 0; i < packet_len; i++) {
    Serial.print(buffer[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
  Serial.print("ASCII: ");
  for (int i = 0; i < packet_len; i++)
    Serial.print((char)buffer[i]);
  Serial.println();
}


