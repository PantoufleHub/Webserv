#!/usr/bin/env python3
import socket
import time

def test_client_timeout():
    print("Testing Client Timeout (408)...")
    print("="*50)
    
    # Connect to server
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        sock.connect(('localhost', 8080))
        print("✓ Connected to localhost:8080")
        
        # Send incomplete request (missing final \r\n\r\n)
        sock.send(b"GET / HTTP/1.1\r\n")
        print("✓ Sent: GET / HTTP/1.1")
        
        sock.send(b"Host: localhost\r\n")
        print("✓ Sent: Host: localhost")
        
        # Don't send the final \r\n\r\n - keep connection open
        print("\n⏳ Waiting for timeout (should happen in ~5 seconds)...")
        
        start_time = time.time()
        
        # Try to receive response (with a longer timeout than server's)
        sock.settimeout(10)
        response = sock.recv(4096)
        
        elapsed = time.time() - start_time
        
        print(f"\n✓ Response received after {elapsed:.2f} seconds")
        print("\nResponse:")
        print("-" * 50)
        print(response.decode())
        print("-" * 50)
        
        # Check if it's a 408
        if b"408" in response:
            print("\n✅ SUCCESS: Got 408 Request Timeout!")
            return True
        else:
            print("\n❌ FAIL: Expected 408, but got different response")
            return False
            
    except socket.timeout:
        print("\n❌ FAIL: Socket timed out (server didn't respond)")
        return False
    except Exception as e:
        print(f"\n❌ ERROR: {e}")
        return False
    finally:
        sock.close()
        print("\n✓ Connection closed")

if __name__ == "__main__":
    result = test_client_timeout()
    exit(0 if result else 1)
