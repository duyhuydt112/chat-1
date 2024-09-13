#include    <iostream>
#include    "Transmit.h"

using namespace std;

// Global Variable
std::mutex ExitFlagMutex;


class Sever_Data_Stream : public Transmit_Data{
    private:
        int SeverSocket, ClientSocket; // Create socket prototype
        sockaddr_in ServerAddress;
        static Sever_Data_Stream* Object;
        string NameObject;
        mutex MutexObject;
        char ReceiveBuffer[BUFFER] = {0};
        char SendBuffer[BUFFER] = "Response from client"; 
        int Stage;

        // Create socket connection
        Sever_Data_Stream(int Domain, int TypeSocket, int InternetProtocol, string Name){
            SeverSocket = socket(AF_INET, SOCK_STREAM, InternetProtocol); // InternetProtocol = 0, Domain = AF_INET, TypeSocket = SOCK_STREAM,
            NameObject = Name;
        }
        
    public:
       
        // Prevent Create a Copy of Construction
         Sever_Data_Stream(const Sever_Data_Stream&) = delete;

        // Prevent Assign 
        void operator=(const Sever_Data_Stream&) = delete;

        // Close socket connection
         ~Sever_Data_Stream(){
            close(ClientSocket);
            close(SeverSocket);
            
         }
    
    
        // Setting Sever Socket prototype
        void Config_Socket(short InternetProtocol, uint16_t Port, in_addr_t Adress){
            ServerAddress.sin_family = InternetProtocol; // AF_INET
            ServerAddress.sin_port = htons(Port);
            ServerAddress.sin_addr.s_addr = Adress; //INADDR_ANY
        }

        void Set_Socket(){
            if (setsockopt(SeverSocket, SOL_SOCKET, SO_REUSEADDR, &Option, sizeof(Option)) < 0) {
                cerr << "Set socket option error: " << strerror(errno) << endl;
                close(SeverSocket);
                exit(EXIT_FAILURE);
            }
        }

        // Binding local socket addresss to socket prototype address
        void Sever_Bind(){
            if((bind(SeverSocket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress))) < 0){
                 cerr << "Socket Binding Error: "<< strerror(errno) << endl;
                 close(SeverSocket);
                 exit(EXIT_FAILURE);
             }
         }

 
         // Waiting connect from clients
        void Sever_Listen(int NumberConnect){
            if(listen(SeverSocket, NumberConnect) < 0 ){ // max number connects= 0-4095
            cerr << "Sever Listen Error"<< strerror(errno) << endl;
            close(SeverSocket);
            exit(EXIT_FAILURE);
            }
         }

        // Accept Connection with clients
        void Sever_Accept(){
             if((ClientSocket = accept(SeverSocket, nullptr, nullptr)) < 0){
             cerr << "Sever Accept Error"<< strerror(errno) << endl;
             close(SeverSocket);
             exit(EXIT_FAILURE);
            }
        }
        
        // Send Text
        void Send_Data(int Mode) override{
            if((send(ClientSocket, SendBuffer, strlen(SendBuffer), Mode)) < 0){
                cerr << "Send Message Error"<< strerror(errno) << endl;
                return;
            }
            else
                cout << "Sever: " << SendBuffer << endl;
                strcpy(SendBuffer, "");
        }
        
        // Receive Text
        void Receive_Data(int Mode) override{
            while(true){
                Stage = recv(ClientSocket, ReceiveBuffer, sizeof(ReceiveBuffer), Mode);
                if(Stage  < 0){
                    cerr << "Receive Message Error: "<< strerror(errno) << endl;
                    return;
                }
                 else if(Stage == 0){
                    cout << "Client Disconnect " << endl;
                    close(ClientSocket);
                    close(SeverSocket);
                    exit(0);
                }

                else{
                    cout << "Client: " << ReceiveBuffer << endl;
                    memset(ReceiveBuffer, 0, sizeof(ReceiveBuffer));

                }
            }
            
        }
        
        // Create Method
        static  Sever_Data_Stream* Create_Object(string Name){
            if(nullptr == Object){
                Object = new Sever_Data_Stream(AF_INET, SOCK_STREAM, 0, Name);
            }
            return Object;
        }

        // Enter the sentence and change string
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
                        close(SeverSocket);
                        delete Object;
                        exit(0);
                    } 
                    Send_Data(Mode);
                    
                }
                
                
            }

        }

};



// User Golbal Variable
Sever_Data_Stream* Sever_Data_Stream::Object = nullptr;

int main()
{
    Sever_Data_Stream* Sever = Sever_Data_Stream::Create_Object("Phat_Sever");
    Sever->Config_Socket(AF_INET, PORT, INADDR_ANY);
    Sever->Set_Socket();
    Sever->Sever_Bind();
    Sever->Sever_Listen(5);
    Sever->Sever_Accept();

    // Create Thread for Send and Receive
    thread SendThread(&Sever_Data_Stream::Edit_Send, Sever, 0);
    thread ReceiveThread(&Sever_Data_Stream::Receive_Data, Sever, 0);
    
    // Waiting Thread End
    SendThread.join();
    ReceiveThread.join();
    delete Sever;
    return 0;
}
