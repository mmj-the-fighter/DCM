// API documentation:

// 1. Get the singleton instance of DCM
// static DebugConnectionManager GetInstance()

// 2. Add a command
// void AddRule(string command, ExecuteCommand memberFunc)

// 3. Enable DCM
// void Begin()

// 4. Disable DCM
// void End()

// 5. Set ack string that client gets when connected to DCM
// void SetAcknowledgementString(string s)

// 6. Set logging function
// void SetLogger(DoLog logger)

// 7. Enable/Disable logging
// bool IsLoggingEnabled

// 8. Enable/Disable command dispatch
// bool CanInvokeCommands

//Uncomment this line if you want logging
//#define COMPILE_LOG
using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace dcmns
{
    public enum LogLevel
    {
        DEBUG, INFO, ERROR
    }

    public delegate int ExecuteCommand(string[] commandAndArguments);
    public delegate void DoLog(string message, LogLevel level);

    public class DebugConnectionManager
    {
        private Dictionary<string, ExecuteCommand> ruleDictionary = null;
        private DoLog log = null;
        private TcpListener listener = null;
        private const int port = 64000;
        private const int maxCommandSize = 256;

        private static readonly object syncObj = new object();
        private static DebugConnectionManager instance = null;
        private string acknowledgementString = "Awaiting commands.\nCommand Examples:\nhealth -40\nspin true\nspin false\nspeed 200\naxis 1 1 1\ndone\n";
        private string negativeAck = "NACK\nClient limit reached\nclosing connection\n";

        private Thread listenerThread = null;

        private DebugConnectionManager()
        {
            ruleDictionary = new Dictionary<string, ExecuteCommand>();
        }

        //Part of API - 1
        public static DebugConnectionManager GetInstance()
        {
            lock (syncObj)
            {
                if (instance == null)
                {
                    instance = new DebugConnectionManager();
                }
                return instance;
            }
        }
        //Part of API - 2
        public void AddRule(string command, ExecuteCommand memberFunc)
        {
            ruleDictionary.Add(command, memberFunc);
        }

        private int InvokeMethodForRule(string[] commandAndArguments)
        {
            int resCode = 1;
            if(!CanInvokeCommands) {
                #if COMPILE_LOG
                InvokeInfoLog("Cannot invoke commands");
                #endif
                return resCode;
            }
            ExecuteCommand del = null;
            ruleDictionary.TryGetValue(commandAndArguments[0], out del);
            if (del != null)
            {
                resCode = del.Invoke(commandAndArguments);
            }
            else
            {
                #if COMPILE_LOG
                InvokeErrorLog("Unsupported command " + commandAndArguments[0]);
                #endif
            }
            return resCode;
        }


        //Part of API - 3
        public void Begin()
        {
            DCMCriticalData.CanRunClientThreads = true;
            DCMCriticalData.CanRunListenerThread = true;
            canInvokeCommands = canInvokeCommandsCopy;
            #if COMPILE_LOG
            InvokeInfoLog("Begin()");
            #endif
            try
            {
                listenerThread = new Thread(ListenForClients);
                listenerThread.Start();
            }
            catch (Exception e)
            {
                #if COMPILE_LOG
                InvokeErrorLog(e.Message);
                #endif
            }
        }

        //Part of API - 4
        public void End()
        {
            #if COMPILE_LOG
            InvokeInfoLog("End()");
            #endif
            DCMCriticalData.CanRunClientThreads = false;
            DCMCriticalData.CanRunListenerThread = false;
            canInvokeCommands = false;
        }

        private void ReceiveComandsFromClientAndDispatch(object clientSocketObj)
        {
            Socket client = (Socket)clientSocketObj;
            byte[] commandBytes = new byte[maxCommandSize];
            try
            {
                byte[] acknowledgement =
                    System.
                    Text.
                    Encoding.
                    ASCII.
                    GetBytes(acknowledgementString);
                client.Send(acknowledgement, acknowledgement.Length, 0);
                while(DCMCriticalData.CanRunClientThreads)
                {
                    int receivedNumOfBytes = client.Receive(commandBytes);
                    if (receivedNumOfBytes > 0)
                    {
                        string receivedData =
                            System.
                            Text.
                            Encoding.
                            ASCII.
                            GetString(commandBytes).Substring(0, receivedNumOfBytes);
                        char[] sep = { ' ' };
                        string[] commandWithArgumentsStrArray = receivedData.Split(sep);
                        int length = commandWithArgumentsStrArray.Length;
                        for (int i = 0; i < length; i++)
                        {
                            commandWithArgumentsStrArray[i] = commandWithArgumentsStrArray[i].Trim();
                        }
                        if (commandWithArgumentsStrArray[0].Equals("done")) 
                        {
                            #if COMPILE_LOG
                            InvokeInfoLog("\"done\" command came from client; closing session...");
                            #endif
                            break;
                        }
                        else
                        {
                            #if COMPILE_LOG
                            InvokeInfoLog("Current command: " + receivedData);
                            #endif
                            int rc = InvokeMethodForRule(commandWithArgumentsStrArray);
                            byte[] message = System.Text.Encoding.ASCII.GetBytes("return code " + rc.ToString() + "\n");
                            client.Send(message, message.Length, 0);
                        }
                    }
                    else
                    {
                        #if COMPILE_LOG
                        InvokeInfoLog("No response from client");
                        #endif
                        break;
                    }
                }
                client.Close();
                #if COMPILE_LOG
                InvokeInfoLog("Disconnected client");
                #endif
            }
            catch (Exception e)
            {
                #if COMPILE_LOG
                InvokeErrorLog(e.Message);
                #endif
            }
            #if COMPILE_LOG
            InvokeInfoLog("ReceiveComandsFromClientAndDispatch thread: " + Thread.CurrentThread.Name + " is finished");
            #endif
            DCMCriticalData.DecrementClientThreadCount();
        }

        private void ListenForClients()
        {
            listener = new TcpListener(IPAddress.Any, port);
            listener.Start();
            try
            {
                while (DCMCriticalData.CanRunListenerThread)
                {
                    if (listener.Pending())
                    {
                        if (DCMCriticalData.IsClientThreadCountWithInLimits())
                        {
                            Socket client;
                            client = listener.AcceptSocket();
                            #if COMPILE_LOG
                            InvokeInfoLog("Client connected");
                            #endif
                            Thread receiverThread = new Thread(ReceiveComandsFromClientAndDispatch);
                            receiverThread.Name = client.Handle.ToString();
                            receiverThread.Start(client);
                            DCMCriticalData.IncrementClientThreadCount();
                        }
                        else
                        {
                            Socket client = listener.AcceptSocket();
                            byte[] nack = System.Text.Encoding.ASCII.GetBytes(negativeAck);
                            client.Send(nack, nack.Length, 0);
                            client.Close();
                        }
                    }
                    else
                    {
                        Thread.Sleep(100);
                    }
                }
                listener.Stop();
            }
            catch (Exception e)
            {
                #if COMPILE_LOG
                InvokeErrorLog(e.Message);
                #endif
            }
            #if COMPILE_LOG
            InvokeInfoLog("ListenForClients thread is finished");
            #endif
        }

        private class DCMCriticalData
        {
            private static readonly int maxNumOfClientsSupported = 2;
            private static int clientThreadCount = 0;
            private static readonly object syncObjClientThreadCount = new object();

            public static bool IsClientThreadCountWithInLimits()
            {
                lock (syncObjClientThreadCount)
                {
                    return clientThreadCount < maxNumOfClientsSupported;
                }
            }

            public static void IncrementClientThreadCount()
            {
                lock (syncObjClientThreadCount)
                {
                    ++clientThreadCount;
                }
            }
            public static void DecrementClientThreadCount()
            {
                lock (syncObjClientThreadCount)
                {
                    --clientThreadCount;
                }
            }

            private static bool canRunListenerThread = false;
            private static readonly object syncObjCanRunListenerThread = new object();
            public static bool CanRunListenerThread
            {
                get
                {
                    lock (syncObjCanRunListenerThread)
                    {
                        return canRunListenerThread;
                    }
                }
                set
                {
                    lock (syncObjCanRunListenerThread)
                    {
                        canRunListenerThread = value;
                    }
                }
            }

            private static bool canRunClientThreads = false;
            private static readonly object syncObjCanRunClientThreads = new object();
            public static bool CanRunClientThreads
            {
                get
                {
                    lock (syncObjCanRunClientThreads)
                    {
                        return canRunClientThreads;
                    }
                }
                set
                {
                    lock (syncObjCanRunClientThreads)
                    {
                        canRunClientThreads = value;
                    }
                }
            }
        }
        
        //Part of API - 5
        public void SetAcknowledgementString(string s)
        {
            acknowledgementString = s;
        }

        //Part of API - 6
        public void SetLogger(DoLog logger)
        {
            log = logger;
        }

        private void InvokeInfoLog(string message) 
        { 
            if(IsLoggingEnabled && log != null)
            {
                log.Invoke(message, LogLevel.INFO);
            }
        }

        private void InvokeErrorLog(string message)
        {
            if (IsLoggingEnabled && log != null)
            {
                log.Invoke(message, LogLevel.ERROR);
            }
        }

        private void InvokeLog(string message, LogLevel level)
        {
            if (IsLoggingEnabled && log != null)
            {
                log.Invoke(message, level);
            }
        }

        private bool isLoggingEnabled = true;

        //Part of API -  7
        public bool IsLoggingEnabled
        {
            get { return isLoggingEnabled; }
            set { isLoggingEnabled = value; }
        }

        private bool canInvokeCommands = true;
        private bool canInvokeCommandsCopy = true;

        //Part of API - 8
        public bool CanInvokeCommands
        {
            get { return canInvokeCommands; }
            set { canInvokeCommandsCopy = canInvokeCommands = value; }
        }
    }
}