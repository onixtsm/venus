import paho.mqtt.client as mqtt
import json


##Variable to connect to MQTT host
broker_address = "mqtt.ics.ele.tue.nl"
broker_port = 1883
topic = "/pynqbridge/67/recv"
username = "student129"
password = "xaingu0I"

##Arbitraty data for data dictionaryy
coord_x = 1
coord_y = 5
object_found = 1
object_color = 3
status = 5

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
    "x": coord_x,
    "y": coord_y,
    "obj_found": object_found,
    "color": object_color,
    "status": status,
 }



client = mqtt.Client() #Connect to MQTT client
client.username_pw_set(username, password)  #Set Username and password
client.connect(broker_address, broker_port) #Connect to server
client.on_connect = on_connect #Check if connected succesfuly
client.on_publish = on_publish #Setup publish command to give feedback on message publication

send_msg(client, topic, data)



client.loop_start() #start loop to send data
client.loop_stop()
client.disconnect()