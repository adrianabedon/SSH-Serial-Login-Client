import requests
import json
import websocket
import threading
import subprocess
import serial

### CHANGE THESE ###
subscription_id = "aa22d82d-e270-4bec-b489-fb0cfadcbd59"
resource_group = "aabedon"
vm_name = "testvm"
### END CHANGES ###

arm_endpoint = "https://management.azure.com"
RP_PROVIDER = "Microsoft.SerialConsole"
connection_url = "%s/subscriptions/%s/resourcegroups/%s/providers/Microsoft.Compute/virtualMachines/%s/providers/%s/serialPorts/0/connect?api-version=2018-05-01" % (arm_endpoint, subscription_id, resource_group, vm_name, RP_PROVIDER)
token = subprocess.run(["az","account","get-access-token","--query","accessToken","-o","tsv"], stdout=subprocess.PIPE).stdout.decode().rstrip("\n").rstrip("\r")
application_json_format = "application/json"
serial_console_ux_prod_url = "https://portal.serialconsole.azure.com"

headers = {'authorization' : "Bearer " + token, 'accept' : application_json_format, 'origin' : serial_console_ux_prod_url, 'content-type' : application_json_format, 'content-length' : "0"}

if __name__ == "__main__":
    result = requests.post(connection_url, headers = headers)
    try:
        websocket_URL = json.loads(result.text)["connectionString"]
    except KeyError:
        print(result.text)
        print("Please run az login")
        exit()
    print(websocket_URL)
    
    ws = websocket.WebSocket(skip_utf8_validation=True)
    ws.connect(websocket_URL + "?authorization=" + token)
    
    #launch socat to create virtual serial port
    socat_process = subprocess.Popen(["socat", "-d", "-d", "pty,raw,echo=0", "pty,raw,echo=0"], stderr = subprocess.PIPE, stdout=subprocess.PIPE)

    first_serial = ""
    second_serial = ""

    output = ""
    for line in socat_process.stderr:
        output += line.decode()
        try:
            first_start = output.index("/dev/pts/")
            first_end = output.index("\n", first_start)

            first_serial = output[first_start:first_end]

            second_start = output.index("/dev/pts/", first_end)
            second_end = output.index("\n", second_start)

            second_serial = output[second_start:second_end]
            break
        except ValueError:
            pass
    
    #open serial device
    ser = serial.Serial(first_serial)
    def listen_for_keys():
        while(True):
            c = ser.read()
            ws.send_binary(c)
    
    th = threading.Thread(target=listen_for_keys, args=()).start()
    i = 0
    first_message = True
    while True:
        try:
            opcode, data = ws.recv_data()
            if first_message:
                print("### Connect the SSH login client to this serial device: ", second_serial, " ###")
            first_message = False
            #print(i, opcode, type(data))
            ser.write(data)
            i+=1
        except:
            print("Closing")
            ser.close()
            socat_process.terminate()
            break
