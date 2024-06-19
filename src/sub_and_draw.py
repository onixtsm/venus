import tkinter as tk
from collections import defaultdict
import threading
import paho.mqtt.client as mqtt
import json

class TrailPanel(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)
        self.master = master
        self.pack(fill=tk.BOTH, expand=True)
        
        self.trail_points = set()
        self.detected_obstacles = defaultdict(lambda: "black")
        self.canvas = tk.Canvas(self, bg='darkgray')
        self.canvas.pack(fill=tk.BOTH, expand=True)
        
        self.canvas.bind("<Configure>", self.on_resize)
        
        # Add a button to quit the mainloop
        self.quit_button = tk.Button(self, text="TU/exit", command=self.quit_mainloop)
        self.quit_button.pack()
        
    def add_point(self, point):
        self.trail_points.add(point)
        self.repaint()
    
    def add_obstacle(self, point, color):
        self.detected_obstacles[point] = color
        self.repaint()
    
    def has(self, point):
        return point in self.trail_points or point in self.detected_obstacles
    
    def clear_all(self):
        self.trail_points.clear()
        self.detected_obstacles.clear()
        self.repaint()
    
    def repaint(self):
        self.canvas.delete("all")
        self.draw_trail_points()
        self.draw_obstacles()
        
    def draw_trail_points(self):
        for point in self.trail_points:
            self.canvas.create_oval(point[0], point[1], point[0]+5, point[1]+5, fill="orange")
    
    def draw_obstacles(self):
        for point, color in self.detected_obstacles.items():
            self.canvas.create_oval(point[0], point[1], point[0]+5, point[1]+5, fill=color)
    
    def on_resize(self, event):
        self.repaint()
    
    def quit_mainloop(self):
        self.master.quit()  # This will stop the main loop

panel = None

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe("some/topic")

def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))
    # Here you can add points or obstacles to the panel based on the MQTT messages

def start_mqtt(broker_address, broker_port, username, password, topic):
    client = mqtt.Client()
    client.username_pw_set(username, password)
    client.connect(broker_address, broker_port)
    client.subscribe(topic)
    client.on_connect = on_connect
    client.on_message = on_message
    client.loop_forever()
    
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
    panel.add_point((data['robot_x'], data['robot_y']))
    with open('data.json', 'w') as json_file:
        json.dump(data, json_file, indent=4)

def main():
    global panel
    root = tk.Tk()
    root.geometry("600x400")
    panel = TrailPanel(master=root)
    
    # MQTT Configuration
    broker_address = "mqtt.ics.ele.tue.nl"
    broker_port = 1883
    username = "Student57"
    password = "Thu0oogo"
    topic = "#"

    # Start the MQTT client in a separate thread
    mqtt_thread = threading.Thread(target=start_mqtt, args=(broker_address, broker_port, username, password, '/pynqbridge/29/send'))
    mqtt_thread.daemon = True
    mqtt_thread.start()
    
    # Start the MQTT client in a separate thread
    #mqtt_thread_2 = threading.Thread(target=start_mqtt, args=(broker_address, broker_port, username, password, '/pynqbridge/b/send'))
    #mqtt_thread_2.daemon = True
    #mqtt_thread_2.start()
    
    root.mainloop()

if __name__ == "__main__":
    main()
