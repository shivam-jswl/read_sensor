Details of topics and data types

    Attribute (Topic Name) (message type):

        SUPPLY_VOLTAGE (/supply_voltage) (std_msgs::msg::UInt16)
        ENV_TEMP (/env_temp) (std_msgs::msg::Int16)
        YAW (/yaw) (std_msgs::msg::Int16)
        PITCH (/pitch) (std_msgs::msg::Int16)
        ROLL (/roll) (std_msgs::msg::Int16)



User can start or stop sensor by calling service "/userStart" and "/userStop" respectively, with service of type std_srvs::srv::Empty.

One can also provide interval to the sensor server by setting the "interval" parameter of node "/read_sensor" in milliseconds, and again starting the sensor server.
By default interval is set to 10ms.


