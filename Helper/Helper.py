
from enum import Enum
import websocket


#connect to websocket server
ws = websocket.WebSocket()
ws.connect("ws://192.168.137.102")
print("Connected!\n")

ws.send_binary([0])


ws.close()
