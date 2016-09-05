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
public class DataRecord {
    
    private int id;
    private String json;
    private byte[] serialize;
    
    public DataRecord(int id, String json){
        this.id = id;
        this.json = json;
        
        serialize = serialize();             
    }
    
    public byte[] getData(){
        return serialize;
    }
    
    private byte[] serialize(){
        byte[] sizeData = json.getBytes();
                
        ByteBuffer idData = ByteBuffer.allocate(4);
        idData.putInt(id);
        byte[] idData4Bytes = idData.array();      
        
        byte[] toReturn = new byte[4+sizeData.length];
        
        toReturn[0] = idData4Bytes[0];
        toReturn[1] = idData4Bytes[1];
        toReturn[2] = idData4Bytes[2];
        toReturn[3] = idData4Bytes[3];
        
        int j = 0;
        for(int i = 4; i < toReturn.length ; i++){
            toReturn[i] = sizeData[j];
            j++;
        }
        
        return toReturn;
    }
}
