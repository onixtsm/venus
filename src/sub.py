import paho.mqtt.client as mqtt
import json

##Variable to connect to MQTT host
broker_address = "mqtt.ics.ele.tue.nl"
broker_port = 1883
topic = "#"
username = "Student57"
password = "Thu0oogo"


#Setup what to do when receiving a message
def on_message(client,userdata, message):
    #print(f"Message received: {message.payload.decode()} on topic {message.topic}")
    json_string = message.payload.decode('utf-8')
    data = json.loads(json_string)
    print(f"Coordinates: x={data['robot_x']} y={data['robot_y']}")
    print("Object found:", data['obstacle_type'])
    print("Object color:", data['obstacle_color'])
    print("Status:", data['robot_status'])
    print("Status:", data['obstacle_x'])
    print("Status:", data['obstacle_y'])
    with open('data.json', 'w') as json_file:
        json.dump(data, json_file, indent=4)


#check if connection is succesful
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connectioned succesfully")
    else:
        print(f"Connecion failed with error code {rc}")

#connection and the other things
client = mqtt.Client()
client.username_pw_set(username, password)
client.connect(broker_address, broker_port)
client.subscribe(topic)
client.on_connect = on_connect



client.on_message = on_message



client.loop_forever()