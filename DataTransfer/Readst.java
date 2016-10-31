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
			//ÿ��seconds������һ�κ���doSomething()	
	}	
			
}

public class Readst 
{

    static String retValue = "000000";
    private SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
    private static MyTimer myTimer;
	private int mutex = 0;
    
    //�������
    Service service = new Service(); 

    //��ʼ������
    public void init() 
    {
        try
        {
        	// ֱ��ȡ��COM1�˿�
            CommPortIdentifier portId = CommPortIdentifier.getPortIdentifier("COM1");
            
            System.out.println(portId.getName() + ":����");
            
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
        			// �ڶ������÷���ֵ����ΪString�ķ��� 
        			call.setOperationName(new QName("http://tempuri.org/", "getpcontrol")); 
        			call.setSOAPActionURI("http://tempuri.org/getp4control"); 
        			
        			while(true)
        			{
        				String retVal = (String) call.invoke(new Object[] {});
        				if(retVal.equals("window"))
        				{
        					System.out.println("����ͨ��"); 
						
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
                            			// �ڶ������÷���ֵ����ΪString�ķ��� 
                            			call.setOperationName(new QName("http://tempuri.org/", "receivepr")); 
                            			call.setSOAPActionURI("http://tempuri.org/receivepr"); 
                            			call.addParameter(new QName("http://tempuri.org/", "data"),// �����name��Ӧ.NET��Ĳ������� 
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
                //����˿ڱ�ռ�þ��׳�����쳣
                e.printStackTrace();
                
            }

            try 
            {
            	//��COM1��ȡ����
                inputStream = serialPort.getInputStream();
                    
            } 
            catch (IOException e) 
            {
            
            }

            try 
            {
            	//��Ӽ�����
                serialPort.addEventListener(this);
                
            } 
            catch (TooManyListenersException e) 
            {
            }

            serialPort.notifyOnDataAvailable(true);
            
            /*
             * ����������������,���������¼�
             */
            try 
            {
                serialPort.setSerialPortParams(9600,//������
                        SerialPort.DATABITS_8,//����λ��
                        SerialPort.STOPBITS_1,//ֹͣλ
                        SerialPort.PARITY_NONE);//У��
            } 
            catch (UnsupportedCommOperationException e) 
            {
            }
            //readThread = new Thread(this);
            //readThread.start();
            control = new Thread(new GetControl());
            control.start();
            //�����߳�
        }

        public void run() 
        {
            try 
            {
                Thread.sleep(100000);
                serialPort.close();
                System.out.println("COM1:�ر�");
                //�趨30���˿ڹرգ�������֮����
            } 
            catch (InterruptedException e) 
            {
            }
        }

        /**
         * BI -ͨѶ�ж�. CD -�ز����. CTS -�������. DATA_AVAILABLE -�����ݵ���. DSR -�����豸׼����.
         * FE -֡����. OE -��λ����. OUTPUT_BUFFER_EMPTY -��������������. PE -��żУ���. RI -
         * ����ָʾ. һ����õľ���DATA_AVAILABLE--���������ݵ����¼���
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
                            	
                            	//����web service
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
                                    System.out.println("���Դ�����:" + temp);
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
        //��ʼ����ʱ��
        myTimer = new MyTimer(1); 
        myTimer.start();
        reader.init();
        reader.retValue = "0";
    }
}
//�˳���ֻ���ڼ���COM1�˿ڣ�������ִ��ʱ����COM1�˿ڣ��ȴ�30��󣬶˿�COM1�رգ�������֮�ر�
//Read Ϊ�ڲ��࣬�������Թ��ö�ȡ�������ַ�����Ϣ
//��Readstr �ж���һ����̬����static String str = "000000";
//Ȼ���ڶ������ݺ�str = new String(readBuffer).trim();
//����������һ���¼������󣬽�str�е�ֵ������Ӧ��ֵ(����˵�������ֵ��ʾ���������)

