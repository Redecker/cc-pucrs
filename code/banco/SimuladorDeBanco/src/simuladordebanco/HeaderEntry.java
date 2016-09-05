/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package simuladordebanco;

import java.nio.ByteBuffer;

/**
 *
 * @author User
 */
public class HeaderEntry {
    
    private short initialByte;
    private short size;
    private byte[] serialize;
    
    public HeaderEntry(short initialByte, short size){
        this.size = size;
        this.initialByte = initialByte;
        
        serialize = serialize();
    }
    
    public byte[] getHeader(){
        return serialize;
    }
    
    private byte[] serialize(){        
        ByteBuffer sizeData = ByteBuffer.allocate(2);
        sizeData.putShort(size);
        byte[] sizeData2Byte = sizeData.array();
        
        ByteBuffer initialData = ByteBuffer.allocate(2);
        initialData.putShort(initialByte);
        byte[] initialData2Byte = initialData.array();        
        
//        for (int i = 0 ; i < 2 ; i++){
//            System.out.println(Integer.toBinaryString(0xFFFF & firstByte[i]));
//        }
//        
//        System.out.println("");
//        
//        for (int i = 0 ; i < 2 ; i++){
//            System.out.println(Integer.toBinaryString(0xFFFF & secondByte[i]));
//        }

        byte[] aux = new byte[4];
        
        aux[0] = initialData2Byte[0];
        aux[1] = initialData2Byte[1];
        aux[2] = sizeData2Byte[0];
        aux[3] = sizeData2Byte[1];
        
        return aux;        
    }
    
}
