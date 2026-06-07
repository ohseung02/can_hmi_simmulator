import asyncio
import websockets

async def listen():
    try:
        async with websockets.connect("ws://127.0.0.1:18080/ws") as ws:
            print("Connected")
            while True:
                msg = await ws.recv()
                if "STATISTICS" in msg:
                    print("Received STATS:", msg)
                    break
    except Exception as e:
        print("Error:", e)

if __name__ == "__main__":
    asyncio.run(listen())
