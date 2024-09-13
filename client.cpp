#include    <iostream>
#include    "Transmit.h"

using namespace std;

class Client_Data_Stream : public Transmit_Data{
    private: 
        int ClientSocket; 
        sockaddr_in ClientAddress;
        char ReceiveBuffer[BUFFER] = {0};
        char SendBuffer[BUFFER] = "Response from client"; 
        mutex MutexObject;
        int Stage;
    public:

        // Config Client Socket
        void Config_Socket(short InternetProtocol, uint16_t Port, in_addr_t Adress){
            ClientAddress.sin_family = InternetProtocol; // AF_INET
            ClientAddress.sin_port = htons(Port);
            ClientAddress.sin_addr.s_addr = Adress; //INADDR_ANY
            
        }
        
        // Create Socket
        Client_Data_Stream(int Domain, int TypeSocket, int InternetProtocol){
            ClientSocket = socket(Domain, TypeSocket, InternetProtocol);// InternetProtocol = 0, Domain = AF_INET, TypeSocket = SOCK_STREAM,
        }
        //using Sever_Data_Stream::Sever_Data_Stream
        
        ~Client_Data_Stream(){
            close(ClientSocket);
            
        }
        
        void Client_Conneted(){
            if((connect(ClientSocket, (struct sockaddr*)&ClientAddress, sizeof(ClientAddress))) < 0){
                cerr << "Client Connected Error: "<< strerror(errno) << endl;
                exit(EXIT_FAILURE);
            }
        }
        
        // Send Text
        void Send_Data(int Mode) override{
            if((send(ClientSocket, SendBuffer, strlen(SendBuffer), Mode)) < 0){
                cerr << "Send Message Error: "<< strerror(errno) << endl;
                return;
            }
            else
                cout << "Client: " << SendBuffer << endl;
                strcpy(SendBuffer, "");
        }
        

        void Edit_Send(int Mode) override{
            while(true){
                unique_lock<mutex> lock(MutexObject);
                cin.getline(SendBuffer, BUFFER);
                Input_Clear::Clear_Input_CommandLine();
                if(cin.fail()){
                    cerr << "Input Error: "<< strerror(errno) << endl;
                    cin.clear(); 
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    return;
                }
                else{
                    if(strcmp(SendBuffer, "~") == 0){
                        close(ClientSocket);
                        exit(0);
                    } 
                    Send_Data(Mode);
                }
                 
                
            }
        }
        // Receive Text
        void Receive_Data(int Mode) override{
            while(true){
                Stage = recv(ClientSocket, ReceiveBuffer, sizeof(ReceiveBuffer), Mode);
                if(Stage < 0){
                    cerr << "Receive Message Error: "<< strerror(errno) << endl;
                    return;
                }
                else if(Stage == 0){
                    cout << "Sever Disconnect " << endl;
                    close(ClientSocket);
                    exit(0);
                }

                else{
                    cout << "Sever: " << ReceiveBuffer << endl;
                    memset(ReceiveBuffer, 0, sizeof(ReceiveBuffer));

                }
                    

            }
            
        }
};  



// Global Variable
std::mutex ExitFlagMutex;

int main() 
{
    Client_Data_Stream* Client = new Client_Data_Stream(AF_INET, SOCK_STREAM, 0);
    Client->Config_Socket(AF_INET, PORT, INADDR_ANY);
    Client->Client_Conneted();
        // Create Thread for Send and Receive
    thread SendThread(&Client_Data_Stream::Edit_Send, Client, 0);
    thread ReceiveThread(&Client_Data_Stream::Receive_Data, Client, 0);
    
    // Waiting Thread End
    SendThread.join();
    ReceiveThread.join();
    delete Client;


    return 0;
}
