#include<rclcpp/rclcpp.hpp>
#include<vector>
#include<chrono>
#include<memory>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sstream>
#include<string.h>

#include<std_msgs/msg/int16.hpp>
#include<std_msgs/msg/u_int16.hpp>
#include<std_srvs/srv/empty.hpp>


#define _SERVER_IP "127.0.0.1"
const short PORT = 2000;

class read_sensor : public rclcpp::Node{

    public:
        read_sensor(): Node("read_sensor_Node"){
            
            using namespace std::chrono_literals;

            _pub_env_temp = create_publisher<std_msgs::msg::Int16>("env_temp", 10);
            _pub_yaw = create_publisher<std_msgs::msg::Int16>("yaw", 10);
            _pub_pitch = create_publisher<std_msgs::msg::Int16>("pitch", 10);
            _pub_roll = create_publisher<std_msgs::msg::Int16>("roll", 10);
            _pub_supply_voltage = create_publisher<std_msgs::msg::UInt16>("supply_voltage", 10);

            _userStartClient = create_client<std_srvs::srv::Empty>("userStart");
            _userStartServer = create_service<std_srvs::srv::Empty>("userStart", std::bind(&read_sensor::userStartCallback, this, std::placeholders::_1, std::placeholders::_2));
            _userStopClient = create_client<std_srvs::srv::Empty>("userStop");
            _userStopServer = create_service<std_srvs::srv::Empty>("userStop", std::bind(&read_sensor::userStopCallback, this, std::placeholders::_1, std::placeholders::_2));



            declare_parameter<uint16_t>("interval", 10);
            _interval = get_parameter("interval").as_int();

            _timer = create_wall_timer(1s, std::bind(&read_sensor::timeCallback, this));

            
            connectToSensorServer();
            //calling sending start signal when node starts
            startSensor(_interval);
        }
        ~read_sensor(){
            //stopping sensor at the end of Node lifetime
            stopSensor();
            close(_socket_fd);
        }
    
    private:
        rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr _pub_supply_voltage;
        rclcpp::Publisher<std_msgs::msg::Int16>::SharedPtr _pub_env_temp;
        rclcpp::Publisher<std_msgs::msg::Int16>::SharedPtr _pub_yaw;
        rclcpp::Publisher<std_msgs::msg::Int16>::SharedPtr _pub_pitch;
        rclcpp::Publisher<std_msgs::msg::Int16>::SharedPtr _pub_roll;

        std_msgs::msg::UInt16 _value_supply_voltage;
        std_msgs::msg::Int16 _value_env_temp;
        std_msgs::msg::Int16 _value_yaw;
        std_msgs::msg::Int16 _value_pitch;
        std_msgs::msg::Int16 _value_roll;

        uint16_t _interval;

        rclcpp::TimerBase::SharedPtr _timer;
        rclcpp::Client<std_srvs::srv::Empty>::SharedPtr _userStartClient;
        rclcpp::Client<std_srvs::srv::Empty>::SharedPtr _userStopClient;
        rclcpp::Service<std_srvs::srv::Empty>::SharedPtr _userStartServer;
        rclcpp::Service<std_srvs::srv::Empty>::SharedPtr _userStopServer;

        bool _SENOR_STATUS_FLAG;

        int _socket_fd;


        void timeCallback(){
            reciveMessage();
            _pub_supply_voltage->publish(_value_supply_voltage);
            _pub_env_temp->publish(_value_env_temp);
            _pub_yaw->publish(_value_yaw);
            _pub_pitch->publish(_value_pitch);
            _pub_roll->publish(_value_roll);


            RCLCPP_INFO(get_logger(), "%d", _interval);

        }

        void userStartCallback(const std::shared_ptr<std_srvs::srv::Empty::Request> req, std::shared_ptr<std_srvs::srv::Empty::Response> res){
            _interval = get_parameter("interval").as_int();
            startSensor(_interval);
        }

        void userStopCallback(const std::shared_ptr<std_srvs::srv::Empty::Request> req, std::shared_ptr<std_srvs::srv::Empty::Response> res){
            stopSensor();
        }

        void connectToSensorServer(){
            
            struct sockaddr_in address;
            if((_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0){
                RCLCPP_INFO(get_logger(), "Unable to create socket");
            }
            address.sin_family = AF_INET;
            address.sin_port = htons(PORT);
            if(inet_pton(AF_INET, _SERVER_IP, &address.sin_addr) <= 0){
                RCLCPP_INFO(get_logger(), "Unable to create address.");
            }

            RCLCPP_INFO(get_logger(), "Connecting to sensor server.....");
            if(connect(_socket_fd, (struct sockaddr *)&address, (socklen_t)sizeof(address)) == -1){
                RCLCPP_INFO(get_logger(), "Unable to connect sensor server.");
            }
        }

        void reciveMessage(){
            // put payload data recieved from TCP Server in following
            // _value_supply_voltage, _value_env_temp, 
            // _value_yaw, _value_pitch,_value_roll

            char messageFromSensor[30]={0};
            messageFromSensor[30] = 0;

            RCLCPP_INFO(get_logger(), "Reading from sensor....");
            if(read(_socket_fd, messageFromSensor, 29) == -1){
                RCLCPP_INFO(get_logger(), "Unable to read data from sensor");
            }
            RCLCPP_INFO_STREAM(get_logger(), "Message from sensor is \""<<messageFromSensor<<"\"");

            std::string buffer = messageFromSensor;

            _value_supply_voltage.data = stoi(buffer.substr(5, 4), nullptr, 16);
            _value_env_temp.data = stoi(buffer.substr(9, 4), nullptr, 16);
            _value_yaw.data = stoi(buffer.substr(13, 4), nullptr, 16);
            _value_pitch.data = stoi(buffer.substr(17, 4), nullptr, 16);
            _value_roll.data =stoi(buffer.substr(21, 4), nullptr, 16);


            // _value_supply_voltage.data = stoi((std::to_string(stoi(buffer.substr(3, 2), 0, 16)) + std::to_string(stoi(buffer.substr(5, 2), 0, 16))));
            // _value_env_temp.data = stoi((std::to_string(stoi(buffer.substr(7, 2), 0, 16)) + std::to_string(stoi(buffer.substr(9, 2), 0, 16))));
            // _value_yaw.data = stoi((std::to_string(stoi(buffer.substr(11, 2), 0, 16)) + std::to_string(stoi(buffer.substr(13, 2), 0, 16))));
            // _value_pitch.data = stoi((std::to_string(stoi(buffer.substr(15, 2), 0, 16)) + std::to_string(stoi(buffer.substr(17, 2), 0, 16))));
            // _value_roll.data = stoi((std::to_string(stoi(buffer.substr(19, 2), 0, 16)) + std::to_string(stoi(buffer.substr(21, 2), 0, 16))));

        }
        void startSensor(const uint16_t& interval){
            //send commond tohexa (# + 03 + interval + \r + \n")

            std::string msg;
            msg = (std::string)"03" + std::to_string(interval) + (std::string)"\r\n";
            std::stringstream Tohex;
            Tohex <<"#";
            for(char s: msg)
                Tohex << std::hex <<std::setfill('0') <<std::setw(2) <<(int)s;
            msg = Tohex.str();
            const char * messageToSensor = msg.c_str();

            RCLCPP_INFO(get_logger(), "Sending singnal....");

            if(send(_socket_fd, messageToSensor, strlen(messageToSensor), 0)== -1){
                RCLCPP_INFO(get_logger(), "Unable to send data to sensor");
            }

            RCLCPP_INFO(get_logger(), "Message send to sensor is \"%s", messageToSensor);
        }

        void stopSensor(){
            //send cammond toHex (# + 09 + \r + \n)

            std::string msg;
            msg = (std::string)"09" + (std::string)"\r\n";
            std::stringstream Tohex;
            Tohex <<"#";
            for(char s: msg)
                Tohex << std::hex << (int)s;
            msg = Tohex.str();
            const char * messageToSensor = msg.c_str();
            send(_socket_fd, messageToSensor, strlen(messageToSensor), 0);

            RCLCPP_INFO(get_logger(), "Message send to sensor is \"%s", messageToSensor);
        }

        
};


int main(int argc, char** argv){
    rclcpp::init(argc, argv);
    std::shared_ptr<read_sensor> n = std::make_shared<read_sensor>();

    rclcpp::spin(n);
    rclcpp::shutdown();

    return 0;
}