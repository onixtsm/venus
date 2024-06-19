import paho.mqtt.client as mqtt
import json
import time


##Variable to connect to MQTT host
broker_address = "mqtt.ics.ele.tue.nl"
broker_port = 1883
topic = "/pynqbridge/67/recv"
username = "Student129"
password = "xaingu0I"

##Arbitraty data for data dictionaryy
coord_x = 2
coord_y = 30
object_found = 3
object_color = 4
status = 0
obstacle_x= 1
obstacle_y= 3

##Check if connection worked
def on_connect(client, userdata, flags,rc):
    if rc == 0:
        print("Connected successfully")
        
    else:
        print(f"Connect failed with code {rc}")

#check if message is published correctly
def on_publish(client, userdata, mid):
        print("publish successfully")
        

#function to send data
def send_msg(client, topic, data):
    json_string  = json.dumps(data)
    byte_data = json_string.encode('utf-8')
   
    client.publish(topic, byte_data)
       

#data
data = {  
    "robot_x": coord_x,
    "robot_y": coord_y,
    "robot_status": status,
    "obstacle_x": obstacle_x,
    "obstacle_y": obstacle_y,
    "obstacle_type": object_found,
    "obstacle_color": object_color,
    
 }



client = mqtt.Client() #Connect to MQTT client
client.username_pw_set(username, password)  #Set Username and password
client.connect(broker_address, broker_port) #Connect to server
client.on_connect = on_connect #Check if connected succesfuly
client.on_publish = on_publish #Setup publish command to give feedback on message publication

x = 0
while x != 10:  
    send_msg(client, topic, data)
    data['robot_status'] += 1
    x += 1
    time.sleep(0.5)




client.loop_start() #start loop to send data
client.loop_stop()
client.disconnect()
