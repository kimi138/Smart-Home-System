/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.MalformedURLException;
import java.rmi.RemoteException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.TooManyListenersException;

import javax.comm.CommPortIdentifier;
import javax.comm.PortInUseException;
import javax.comm.SerialPort;
import javax.comm.SerialPortEvent;
import javax.comm.SerialPortEventListener;
import javax.comm.UnsupportedCommOperationException;

import javax.xml.namespace.QName; 
import javax.xml.rpc.ParameterMode; 
import javax.xml.rpc.ServiceException;

import org.apache.axis.client.Call; 
import org.apache.axis.client.Service; 
import org.apache.axis.encoding.XMLType; 


import java.util.Timer;
import java.util.TimerTask;

class MyTimer
{	
	private final Timer timer = new Timer();	
	private int seconds;	
	public int counter = 0;
	
	public MyTimer(int seconds) 
	{		
		this.seconds = seconds;	
	}	
	private void doSomething() 
	{		
		counter++;
		//System.out.println("counter="+counter);
	}	
	public void start() 
	{		
		timer.schedule(new TimerTask(){			
			public void run(){				
				doSomething();		
			}	
		},0,this.seconds*1000);       
			//每隔seconds秒运行一次函数doSomething()	
	}	
			
}

public class Readst 
{

    static String retValue = "000000";
    private SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
    private static MyTimer myTimer;
	private int mutex = 0;
    
    //定义服务
    Service service = new Service(); 

    //初始化函数
    public void init() 
    {
        try
        {
        	// 直接取得COM1端口
            CommPortIdentifier portId = CommPortIdentifier.getPortIdentifier("COM1");
            
            System.out.println(portId.getName() + ":开启");
            
            @SuppressWarnings("unused")
            Read reader = new Read(portId);
        } 
        catch (Exception ex) 
        {
            ex.printStackTrace();
            
        }
    }

    class Read implements Runnable, SerialPortEventListener
    {

        InputStream inputStream;
        SerialPort serialPort;
        Thread readThread;
        Thread control;
		Call call;
        
        class GetControl implements Runnable
        {
        	public void run() 
            {
        		Call call;
        		try 
        		{
        			call = (Call) service.createCall();
        			call.setTargetEndpointAddress(new java.net.URL("http://192.168.4.59:9977/CalService.asmx")); 
        			call.setUseSOAPAction(true); 
        			call.setReturnType(new QName("http://www.w3.org/2001/XMLSchema","string")); 
        			// 第二种设置返回值类型为String的方法 
        			call.setOperationName(new QName("http://tempuri.org/", "getpcontrol")); 
        			call.setSOAPActionURI("http://tempuri.org/getp4control"); 
        			
        			while(true)
        			{
        				String retVal = (String) call.invoke(new Object[] {});
        				if(retVal.equals("window"))
        				{
        					System.out.println("开窗通风"); 
						
        				}
        				Thread.sleep(3000);
        			}
        		} 
        		catch (ServiceException e)
        		{
        			// TODO Auto-generated catch block
        			e.printStackTrace();
        		} catch (MalformedURLException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (RemoteException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
            }
        }

        public Read(CommPortIdentifier portId) throws InterruptedException 
        {
			try
			{
			call = (Call) service.createCall();
                            			call.setTargetEndpointAddress(new java.net.URL("http://192.168.4.59:9977/CalService.asmx")); 
                            			call.setUseSOAPAction(true); 
                            			call.setReturnType(new QName("http://www.w3.org/2001/XMLSchema","string")); 
                            			// 第二种设置返回值类型为String的方法 
                            			call.setOperationName(new QName("http://tempuri.org/", "receivepr")); 
                            			call.setSOAPActionURI("http://tempuri.org/receivepr"); 
                            			call.addParameter(new QName("http://tempuri.org/", "data"),// 这里的name对应.NET里的参数名称 
	                            			XMLType.XSD_STRING, ParameterMode.IN); 
			}
			catch(Exception e)
			{
				e.printStackTrace();
			}
		
            try 
            {
                serialPort = (SerialPort) portId.open("MyReader", 2000);
                
            }
            catch (PortInUseException e) 
            {
                //如果端口被占用就抛出这个异常
                e.printStackTrace();
                
            }

            try 
            {
            	//从COM1获取数据
                inputStream = serialPort.getInputStream();
                    
            } 
            catch (IOException e) 
            {
            
            }

            try 
            {
            	//添加监听器
                serialPort.addEventListener(this);
                
            } 
            catch (TooManyListenersException e) 
            {
            }

            serialPort.notifyOnDataAvailable(true);
            
            /*
             * 侦听到串口有数据,触发串口事件
             */
            try 
            {
                serialPort.setSerialPortParams(9600,//波特率
                        SerialPort.DATABITS_8,//数据位数
                        SerialPort.STOPBITS_1,//停止位
                        SerialPort.PARITY_NONE);//校验
            } 
            catch (UnsupportedCommOperationException e) 
            {
            }
            //readThread = new Thread(this);
            //readThread.start();
            control = new Thread(new GetControl());
            control.start();
            //启动线程
        }

        public void run() 
        {
            try 
            {
                Thread.sleep(100000);
                serialPort.close();
                System.out.println("COM1:关闭");
                //设定30秒后端口关闭，程序随之结束
            } 
            catch (InterruptedException e) 
            {
            }
        }

        /**
         * BI -通讯中断. CD -载波检测. CTS -清除发送. DATA_AVAILABLE -有数据到达. DSR -数据设备准备好.
         * FE -帧错误. OE -溢位错误. OUTPUT_BUFFER_EMPTY -输出缓冲区已清空. PE -奇偶校验错. RI -
         * 振铃指示. 一般最常用的就是DATA_AVAILABLE--串口有数据到达事件。
         */
        public void serialEvent(SerialPortEvent event) 
        {

            switch (event.getEventType()) 
            {
                case SerialPortEvent.BI:
                case SerialPortEvent.OE:
                case SerialPortEvent.FE:
                case SerialPortEvent.PE:
                case SerialPortEvent.CD:
                case SerialPortEvent.CTS:
                case SerialPortEvent.DSR:
                case SerialPortEvent.RI:
                case SerialPortEvent.OUTPUT_BUFFER_EMPTY:
                    break;
                case SerialPortEvent.DATA_AVAILABLE:
                    byte[] readBuffer = new byte[20];
                    char[] cbuf = new char[500];
					//mutex = 1;
                    try 
                    {
                        while (inputStream.available() > 0) 
                        {
                            BufferedReader comReader = new BufferedReader(new InputStreamReader(inputStream));

                            String temp = comReader.readLine().trim();

                            String[] temps = temp.split(":");
                            
                            if(temps.length>3)
                            {
                            	//System.out.println(df.format(new Date())+": ");
                            	//System.out.println("       pressure: " + temps[1].trim());
                            	//System.out.println("       temperature: " + temps[3].trim());
                            	
                            	//调用web service
								if(myTimer.counter == 1)
								{
									mutex = 0;
								}
								
                            	if(myTimer.counter == 5 && mutex == 0)
                            	{
									mutex = 1;
                            		
                            		try 
                            		{
                            			
                            			String retVal = (String) call.invoke(new Object[] { temps[1].trim()+"$" + temps[3].trim()+"$" + df.format(new Date())});
                            			System.out.println(retVal); 
                            		} 
                            		catch (Exception e)
                            		{
                            			// TODO Auto-generated catch block
                            			e.printStackTrace();
                            		} 
                            		myTimer.counter = 0;		
                            	}
                            }
                            
                            if (temp.indexOf("#0") >= 0 || temp.indexOf("#8") >= 0) 
                            {
                                int p = temp.indexOf("#");
                                temp = temp.substring(p + 3, temp.length()).replace(" ", "").trim();
                                try 
                                {
                                    Integer.parseInt(temp);
                                } 
                                catch (Exception e) 
                                {
                                    System.out.println("忽略次数据:" + temp);
                                    retValue = "0";
                                    return;
                                }
                                System.out.println(temp);
                                retValue = String.valueOf(Integer.parseInt(temp.substring(1, 5))) + "." + temp.substring(5, 6);
                                System.out.println(retValue);
                            }
                        }
                    } 
                    catch (IOException e) 
                    {
                    }
                    break;
            }
        }
    }

    public static void main(String[] args) 
    {
        Readst reader = new Readst();
        //初始化定时器
        myTimer = new MyTimer(1); 
        myTimer.start();
        reader.init();
        reader.retValue = "0";
    }
}
//此程序只用于监听COM1端口，当程序执行时开启COM1端口，等待30秒后，端口COM1关闭，程序随之关闭
//Read 为内部类，这样可以共用读取出来的字符串信息
//在Readstr 中定义一个静态变量static String str = "000000";
//然后在读出数据后str = new String(readBuffer).trim();
//这样可以在一个事件触发后，将str中的值赋给相应的值(比如说，让这个值显示在输入框中)

