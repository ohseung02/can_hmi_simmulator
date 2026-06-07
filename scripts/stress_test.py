import asyncio
import websockets
import json
import random
import time

async def stress_test():
    uri = "ws://127.0.0.1:18080/ws"
    print(f"Connecting to {uri}...")
    
    can_ids = ["0x100", "0x101", "0x102", "0x200", "0x300", "0x400"]
    
    try:
        async with websockets.connect(uri) as websocket:
            print("Connected! Switching to publisher mode...")
            await websocket.send(json.dumps({"action": "publisher_mode"}))
            print("Starting stress test. Press Ctrl+C to stop.")
            
            # Background task to drain incoming messages so the server buffer doesn't fill up
            async def drain():
                try:
                    while True:
                        await websocket.recv()
                except:
                    pass
            
            asyncio.create_task(drain())

            msg_count = 0
            start_time = time.time()
            
            while True:
                # Spam all 6 CAN IDs
                # 1. High Priority (Steering Angle)
                await websocket.send(json.dumps({"canId": "0x100", "data": str(-180 + (msg_count % 360))}))
                # 2. Brake
                await websocket.send(json.dumps({"canId": "0x101", "data": str(msg_count % 100)}))
                # 3. Throttle
                await websocket.send(json.dumps({"canId": "0x102", "data": str((msg_count+50) % 100)}))
                # 4. Speed
                await websocket.send(json.dumps({"canId": "0x200", "data": str(msg_count % 250)}))
                # 5. Gear
                await websocket.send(json.dumps({"canId": "0x300", "data": str(msg_count % 4)}))
                # 6. Lowest Priority (Climate Temp)
                await websocket.send(json.dumps({"canId": "0x400", "data": str(18 + (msg_count % 12))}))
                
                # Yield to the event loop so we don't lock up entirely, but keep it very fast
                await asyncio.sleep(0.0005)
                
                msg_count += 6
                
                # Small sleep to yield to event loop but blast as fast as possible
                # 0.0001 sec = 10,000 max msgs/sec
                await asyncio.sleep(0.0001)
                
                if msg_count >= 96:
                    elapsed = time.time() - start_time
                    rate = msg_count / elapsed
                    print(f"Sent {msg_count} messages. Rate: {rate:.2f} msg/sec. Stopping burst to let queue drain.")
                    # Sleep indefinitely so the websocket stays open and server doesn't clean up
                    while True:
                        await asyncio.sleep(1)
                
                if msg_count % 5000 == 0:
                    elapsed = time.time() - start_time
                    rate = msg_count / elapsed
                    print(f"Sent {msg_count} messages. Rate: {rate:.2f} msg/sec")
                    
    except ConnectionRefusedError:
        print("Error: Could not connect to the server. Is CanServer running?")
    except KeyboardInterrupt:
        print("Stress test stopped by user.")

if __name__ == "__main__":
    asyncio.run(stress_test())
