import paho.mqtt.client as mqtt
import json
import tkinter as tk
from tkinter import Canvas
from collections import defaultdict
import threading

class Map(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)
        self.master = master
        self.pack()
        
        self.Map_points = set()
        self.detected_obstacles = defaultdict(lambda: "black")
        self.canvas = Canvas(self, bg='darkgray')
        self.canvas.pack(fill=tk.BOTH, expand=True)
        
        self.canvas.bind("<Configure>", self.on_resize)
        self.canvas.pack(fill=tk.BOTH, expand=True)
        self.pack(fill=tk.BOTH, expand=True)
        
    def add_point(self, point):
        self.Map_points.add(point)
        self.repaint()
    
    def add_obstacle(self, point, color):
        self.detected_obstacles[point] = color
        self.repaint()
    
    def has(self, point):
        return point in self.Map_points or point in self.detected_obstacles
    
    def clear_all(self):
        self.Map_points.clear()
        self.detected_obstacles.clear()
        self.repaint()
    
    def repaint(self):
        self.canvas.delete("all")
        self.draw_points()
        self.draw_obstacles()
        
    def draw_points(self):
        for point in self.Map_points:
            self.canvas.create_oval(point[0], point[1], point[0]+5, point[1]+5, fill="orange")
    
    def draw_obstacles(self):
        for point, color in self.detected_obstacles.items():
            self.canvas.create_oval(point[0], point[1], point[0]+5, point[1]+5, fill=color)
    
    def on_resize(self, event):
        self.repaint()

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
    print(f"Coordinates: x={data['x']} y={data['y']}")
    print("Object found:", data['obj_found'])
    print("Object color:", data['color'])
    print("Status:", data['status'])
    #points.append([data['x'], data['y']])
    map.add_point((data['x'], data['y']))
    #root.mainloop()
    with open('data.json', 'w') as json_file:
        json.dump(data, json_file, indent=4)


#check if connection is succesful
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connectioned succesfully")
    else:
        print(f"Connecion failed with error code {rc}")

#connection and the other things
root = tk.Tk()
root.geometry("600x400")
map = Map(master=root)
#root.mainloop()
client = mqtt.Client()
client.username_pw_set(username, password)
client.connect(broker_address, broker_port)
client.subscribe(topic)
client.on_connect = on_connect
client.on_message = on_message
client.loop_forever()