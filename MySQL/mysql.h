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

  mysql.h - Library for communicating with a MySQL Server over Ethernet.
            
  This header file defines the Connector class for connecting to and
  issuing queries against a MySQL database. You can issue any command
  using SQL statements for inserting or retrieving data.
  
  Dependencies:
  
    - requires the SHA1 code from google code repository. See README.txt
      for more details.
      
  Created by Dr. Charles A. Bell, April 2012.
  
  Motivation: To demonstrate the versatility of the MySQL client protocol
  for adaptation to embedded hardware and to bring MySQL to the Ardiuno
  community.
  
*/
#ifndef mysql_h
#define mysql_h

#include "Arduino.h"
#include <SPI.h>
#include <Ethernet.h>

#define OK_PACKET     0x00
#define EOF_PACKET    0xfe
#define ERROR_PACKET  0xff

// Structure for retrieving the OK packet.
typedef struct {
  int affected_rows;
  int insert_id;
  int server_status;
  int warning_count;
  char message[64];
} ok_packet;

// Structure for retrieving the EOF packet.
typedef struct {
  int warnings;
  int flags;
} eof_packet;

// Structure for retrieving a field (minimal implementation).
typedef struct {
  char *db;
  char *table;
  char *name;
} field_struct;

/**
 * Connector class
 *
 * The connector class permits users to connect to and issue queries
 * against a MySQL database. It is a lightweight connector with the
 * following features.
 *
 *  - Connect and authenticate with a MySQL server (using 'new' 4.1+
 *    protocol).
 *  - Issue simple commands like INSERT, UPDATE, DELETE, SHOW, etc.
 *  - Run queries that return result sets.
 *
 *  There are some strict limitations:
 *
 *  - Queries must fit into memory. This is because the class uses an
 *    internal buffer for building data packets to send to the server.
 *    It is suggested long strings be stored in program memory using
 *    PROGMEM (see cmd_query_P).
 *  - Result sets are read one row-at-a-time.
 *  - The combined length of a row in a result set must fit into
 *    memory. The connector reads one packet-at-a-time and since the
 *    Arduino has a limited data size, the combined length of all fields
 *    must be less than available memory.
 *  - Server error responses are processed immediately with the error
 *    code and text written via Serial.print.
 */
class Connector
{
  public:
    Connector() { buffer = NULL; }
    boolean mysql_connect(IPAddress server, int port, 
                          char *user, char *password);
    boolean cmd_query(const char *query);
    boolean cmd_query_P(const char *query);
    void show_results();
    int get_field(field_struct *fs);
    int get_row();
    int is_connected () { return client.connected(); }
  private:
    byte *buffer; 
    char *server_version;    
    byte seed[20];           
    int packet_len;
    
    EthernetClient client;
    
    // Methods for handling packets
    int wait_for_client();
    void send_authentication_packet(char *user, char *password);
    void read_packet();
    void parse_handshake_packet();
    void parse_eof_packet(eof_packet *packet);
    int parse_ok_packet(ok_packet *packet);
    void parse_error_packet();
    boolean run_query(int query_len);
    
    // Utility methods
    boolean scramble_password(char *password, byte *pwd_hash);
    int get_lcb_len(int offset);
    char *read_string(int *offset);
    int read_int(int offset, int size=0);
    void store_int(byte *buff, long value, int size);
    
    // diagnostic methods
    void print_packet();
};

#endif

