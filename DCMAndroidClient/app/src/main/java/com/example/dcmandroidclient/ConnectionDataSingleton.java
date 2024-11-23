package com.example.dcmandroidclient;

import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.UnknownHostException;

public class ConnectionDataSingleton {
    private static ConnectionDataSingleton instance;

    private Socket socket;
    private PrintWriter writer;
    private BufferedReader reader;

    private boolean isConnectedFlag = false;

    private ConnectionDataSingleton() {

    }

    public static ConnectionDataSingleton getInstance() {
        if (instance == null) {
            instance = new ConnectionDataSingleton();
        }
        return instance;
    }

    public boolean connect(String serverIpAddress, int serverPort) {
        boolean retCode = false;
        try {
            socket = new Socket(serverIpAddress, serverPort);
            writer = new PrintWriter(socket.getOutputStream(), true);
            reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            setIsConnected(true);
            retCode = socket.isConnected();
        } catch (UnknownHostException ex) {
            retCode = false;
        } catch (IOException e) {
            retCode = false;
        }
        return retCode;
    }

    public void disconnect() throws IOException {
        writer.close();
        reader.close();
        socket.close();
        setIsConnected(false);
    }

    public PrintWriter getWriter() {
        return writer;
    }

    public BufferedReader getReader() {
        return reader;
    }

    public boolean isConnected() {
        return isConnectedFlag;
    }

    private void setIsConnected(boolean value) {
        isConnectedFlag = value;
    }

}
