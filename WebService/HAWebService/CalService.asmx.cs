using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Services;

using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Web.Services;
using System.Web.Services.Protocols;

using MySql.Data.MySqlClient;

namespace WebServiceTest
{
    /// <summary>
    /// CalService 的摘要说明
    /// </summary>
    [WebService(Namespace = "http://tempuri.org/")]
    [WebServiceBinding(ConformsTo = WsiProfiles.BasicProfile1_1)]
    [System.ComponentModel.ToolboxItem(false)]
    // 若要允许使用 ASP.NET AJAX 从脚本中调用此 Web 服务，请取消注释以下行。 
    // [System.Web.Script.Services.ScriptService]
    [SoapDocumentService(RoutingStyle = SoapServiceRoutingStyle.RequestElement)]
    public class CalService : System.Web.Services.WebService
    {
        private DataSet dsall;
        private static String mysqlcon = "database=sensor_data;Password=123456;User ID=root;server=localhost";//Data Source=MySQL;;charset=utf8";
        private MySqlConnection conn;
        private MySqlDataAdapter mdap;

        private static int mutex = 0;

        //指示是否某个指标超过了阈值
        private static int is_temp_alarm = 0;
        private static int is_press_alarm = 0;
        private static int is_smog_alarm = 0;
        private static int is_humid_alarm = 0;
        private static int is_light_alarm = 0;
        private static int is_hall_alarm = 0;

        //发生警报时的指标值
        private static float alarm_temperature = 0.0f;
        private static float alarm_pressure = 0.0f;
        private static float alarm_humidity = 0.0f;
        private static float alarm_light = 0.0f;

        //各个指标的阈值
        private static float temp_threadhold = 50.0f;
        private static float press_threadhold = 99300.0f;
        private static float humid_threadhold = 35.0f;
        private static float light_threadhold = 20.0f;

        //控制命令字符串
        private static String t_command = "NO CONTROL";
        private static String p_command = "NO CONTROL";
        private static String l_command = "NO CONTROL";
        private static String h_command = "NO CONTROL";

        private static String prev_command = "";

        public CalService()
        {
            conn = new MySqlConnection(mysqlcon);
        }

        //接收并发回串口数据
        [WebMethod]
        public String receive(String s)
        {
            return "OK:" + s;
        }

        //接收压力和温度数据，写入数据库
        [WebMethod]
        public String receivepr(String data)
        {
            //提取数据
            String[] datas = data.Split('$');
            float pressure = float.Parse(datas[0]);
            float temperature = float.Parse(datas[1]);
            String time = datas[2];

            //判断报警
            if (temperature >= temp_threadhold)
            {
                is_temp_alarm = 1;
                alarm_temperature = temperature;
            }
            else 
            {
                is_temp_alarm = 0;
                alarm_temperature = 0.0f;
            }
            if (pressure >= press_threadhold)
            {
                is_press_alarm = 1;
                alarm_pressure = pressure;
            }
            else
            {
                is_press_alarm = 0;
                alarm_pressure = 0.0f;
            }

            //存数据库
            conn.Open();
            MySqlCommand command = new MySqlCommand("INSERT INTO `sensor_data`.`pr_data_table` (`pressure`, `temperature`, `time`) VALUES ('" + pressure + "', '" + temperature + "', '" + time + "');", conn);
            command.ExecuteNonQuery();
            conn.Close();

            return "OK:" + pressure + "-----" + temperature + "-----" + time;
        }

        //接收温度和湿度数据，写入数据库
        [WebMethod]
        public String receivet(String data)
        {
            //提取数据
            String[] datas = data.Split('$');
            float temperature = float.Parse(datas[0]);
            float humidity = float.Parse(datas[1]);
            String time = datas[2];

            //判断报警
            if (temperature >= temp_threadhold)
            {
                is_temp_alarm = 1;
                alarm_temperature = temperature;
            }
            else
            {
                is_temp_alarm = 0;
                alarm_temperature = 0.0f;
            }
            if (humidity <= humid_threadhold)
            {
                is_humid_alarm = 1;
                alarm_humidity = humidity;
            }
            else
            {
                is_humid_alarm = 0;
                alarm_humidity = 0.0f;
            }

            //存数据库
            conn.Open();
            MySqlCommand command = new MySqlCommand("INSERT INTO `sensor_data`.`t_data_table` (`temperature`, `humidity`, `time`) VALUES ('" + temperature + "', '" + humidity + "', '" + time + "');", conn);
            command.ExecuteNonQuery();
            conn.Close();

            return "OK:" + temperature + "-----" + humidity + "-----" + time;
        }

        //接收光强和接近值数据，写入数据库
        [WebMethod]
        public String receivelight(String data)
        {
            //提取数据
            String[] datas = data.Split('$');
            float light = float.Parse(datas[0]);
            float proximity = float.Parse(datas[1]);
            String time = datas[2];

            //判断报警
            if (light <= light_threadhold)
            {
                is_light_alarm = 1;
                alarm_light = light;
            }
            else
            {
                is_light_alarm = 0;
                alarm_light = 0.0f;
            }

            //存数据库
            conn.Open();
            MySqlCommand command = new MySqlCommand("INSERT INTO `sensor_data`.`light_data_table` (`light`, `proximity`, `time`) VALUES ('" + light + "', '" + proximity + "', '" + time + "');", conn);
            command.ExecuteNonQuery();
            conn.Close();

            return "OK:" + light + "-----" + proximity + "-----" + time;
        }

        //接收门磁信息
        [WebMethod]
        public String receivehall(String data)
        {
            if (data.Split('x')[1].Equals(""))
            {
                is_hall_alarm = 0;
            }
            else
            {
                is_hall_alarm = 1;
            }
            return "" + is_hall_alarm;
        }

        //手机调用这个方法读取并判断是否有警告信息
        [WebMethod]
        public String getalarm()
        {
            String alarm = "$";
            if (is_temp_alarm == 1)
            {
                alarm += alarm_temperature + "$";
            }
            else
            {
                alarm += "NO" + "$";
            }
            if(is_press_alarm == 1)
            {
                alarm += alarm_pressure + "$";
            }
            else
            {
                alarm += "NO" + "$";
            }
            if (is_humid_alarm == 1)
            {
                alarm += alarm_humidity + "$";
            }
            else
            {
                alarm += "NO" + "$";
            }
            if (is_light_alarm == 1)
            {
                alarm += alarm_light + "$";
            }
            else
            {
                alarm += "NO" + "$";
            }
            if(is_hall_alarm == 1)
            {
                alarm += "HALL" + "$";
            }
            else
            {
                alarm += "NO" + "$";
            }
            if (is_smog_alarm == 1)
            {
                alarm += "SMOG";
            }
            else 
            {
                alarm += "NO";
            }
            return alarm;
        }

        //查询当前的控制命令
        [WebMethod]
        public String getthcontrol()
        {
            if(prev_command.Equals(t_command))
            {
                return "NO CONTROL";
            }
            prev_command = t_command;
            return t_command;
        }

        //查询当前的控制命令
        [WebMethod]
        public String getpcontrol()
        {
            String ret = p_command;
            p_command = "NO CONTROL";
            return ret;
        }

        //查询当前的控制命令
        [WebMethod]
        public String getlcontrol()
        {
            String ret = l_command;
            l_command = "NO CONTROL";
            return ret;
        }

        //手机调用这个方法发送控制命令
        [WebMethod]
        public String sendthcontrol(String command)
        {
            t_command = command;
            return t_command;
        }

        //手机调用这个方法发送控制命令
        [WebMethod]
        public String sendpcontrol(String command)
        {
            p_command = command;
            return p_command;
        }

        //手机调用这个方法发送控制命令
        [WebMethod]
        public String sendlcontrol(String command)
        {
            l_command = command;
            return l_command;
        }

        //手机调用这个方法查询最新数据
        [WebMethod]
        public String getnewdata()
        {
            conn.Open();
            String newdata = "";
            MySqlCommand cmd1 = new MySqlCommand("select pressure,temperature,time from pr_data_table order by pr_id DESC limit 1", conn);
            MySqlDataReader reader = cmd1.ExecuteReader();
            if(reader.Read())
            {
                newdata += reader.GetFloat(0) + "$" + reader.GetFloat(1) + "$" + reader.GetString(2) + "#";
            }
            reader.Close();

            MySqlCommand cmd2 = new MySqlCommand("select temperature,humidity,time from t_data_table order by t_id DESC limit 1", conn);
            MySqlDataReader reader2 = cmd2.ExecuteReader();
            if (reader2.Read())
            {
                newdata += reader2.GetFloat(0) + "$" + reader2.GetFloat(1) + "$" + reader2.GetString(2) + "#";
            }
            reader2.Close();

            MySqlCommand cmd3 = new MySqlCommand("select light,proximity,time from light_data_table order by light_id DESC limit 1", conn);
            MySqlDataReader reader3 = cmd3.ExecuteReader();
            if (reader3.Read())
            {
                newdata += reader3.GetFloat(0) + "$" + reader3.GetFloat(1) + "$" + reader3.GetString(2) + "#";
            }
            reader3.Close();
            conn.Close();

            return newdata;
        }

        //手机调用这个方法设置温度、气压、光强、湿度的阈值
        [WebMethod]
        public String setThreadhold(float t_threadhold,float p_threadhold,float h_threadhold,float l_threadhold)
        {
            if (!(Math.Abs(t_threadhold) <= 0.000001f))
            {
                temp_threadhold = t_threadhold;
            }
            if (!(Math.Abs(p_threadhold) <= 0.000001f))
            {
                press_threadhold = p_threadhold;
            }
            if (!(Math.Abs(h_threadhold) <= 0.000001f))
            {
                humid_threadhold = h_threadhold;
            }
            if (!(Math.Abs(l_threadhold) <= 0.000001f))
            {
                light_threadhold = l_threadhold;
            }

            return temp_threadhold + "$" + press_threadhold + "$" + humid_threadhold + "$" + light_threadhold;
        }
    }
}
