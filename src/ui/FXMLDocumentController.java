/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package dsvsca;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.application.Platform;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.geometry.Bounds;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextArea;
import javafx.scene.control.TextField;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.AnchorPane;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import javafx.stage.Stage;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;
import javax.sound.sampled.Line;
import javax.sound.sampled.LineEvent;
import javax.sound.sampled.LineListener;

/**
 *
 * @author ryan-pc
 */
public class FXMLDocumentController implements Initializable {
    
    @FXML private AnchorPane calibrateAnchorPane;
    @FXML private Label inputPathLabel;
    @FXML private TextField inputPathTextField;
    @FXML private Label outputPathLabel;
    @FXML private TextField outputPathTextField;
    @FXML private Button calibrateButton;
    @FXML private Button backButton;
    @FXML private Button setInputPathButton;
    @FXML private Button setOutputPathButton;
    @FXML private Button setHRTFPathButton;
    @FXML private TextField HRTFPathTextField;
    @FXML private Label HRTFPathLabel;
    @FXML private Button convertButton;
    @FXML private Button startButton;
    @FXML private Button homeButton;
    @FXML private Circle FL;
    @FXML private Circle FR;
    @FXML private Circle FC;
    @FXML private Circle BL;
    @FXML private Circle BR;
    @FXML private Circle head;
    @FXML private TextArea progressText;
    private static String inPath = "";
    private static String outPath = "";
    private static String HRTFPath = "";
    private static Thread thread = null;
    double orgSceneX, orgSceneY, orgTranslateX, orgTranslateY;
    public enum Channel {
        FL,
        FC,
        FR,
        BL,
        BR,
        LFE
    }
    private static HashMap<Channel, float[]> hashmap = new HashMap<Channel, float[]>();
    float[] FLCoordinates = new float[3];
    float[] FCCoordinates = new float[3];
    float[] FRCoordinates = new float[3];
    float[] BLCoordinates = new float[3];
    float[] BRCoordinates = new float[3];
    
    @FXML
    private void updateHashmap(MouseEvent event, Boolean playSample) {
    	HashMap<Channel, float[]> sampleMap = new HashMap<Channel, float[]>();
    	
        if (event.getSource()==FL) {
            Bounds FLBounds = FL.localToScene(FL.getBoundsInLocal());
            Bounds headBounds = head.localToScene(head.getBoundsInLocal());
            double FLx = ((FLBounds.getMinX() - headBounds.getMinX())/(-35));
            double FLy = ((FLBounds.getMinY() - headBounds.getMinY())/(-43));
            FLCoordinates[0] = (float)FLx;
            FLCoordinates[1] = (float)FLy;
            FLCoordinates[2] = 0;
            hashmap.put(Channel.FL, FLCoordinates);
            sampleMap.put(Channel.FL, FLCoordinates);
            System.out.println("Front Left coordinates are: " + Double.toString(FLx) + ", " + Double.toString(FLy));
        }
        else if (event.getSource()==FC) {
            Bounds FCBounds = FC.localToScene(FC.getBoundsInLocal());
            Bounds headBounds = head.localToScene(head.getBoundsInLocal());
            double FCx = ((FCBounds.getMinX() - headBounds.getMinX())/(8));
            double FCy = ((FCBounds.getMinY() - headBounds.getMinY())/(-43)) - 1;
            FCCoordinates[0] = (float)FCx;
            FCCoordinates[1] = (float)FCy;
            FCCoordinates[2] = 0;
            hashmap.put(Channel.FC, FCCoordinates);
            sampleMap.put(Channel.FC, FCCoordinates);
            System.out.println("Front Center coordinates are: " + Double.toString(FCx) + ", " + Double.toString(FCy));
            
        }
        else if (event.getSource()==FR) {
            Bounds FRBounds = FR.localToScene(FR.getBoundsInLocal());
            Bounds headBounds = head.localToScene(head.getBoundsInLocal());
            double FRx = ((FRBounds.getMinX() - headBounds.getMinX())/(50));
            double FRy = ((FRBounds.getMinY() - headBounds.getMinY())/(43));
            FRCoordinates[0] = (float)FRx;
            FRCoordinates[1] = (float)FRy;
            FRCoordinates[2] = 0;
            hashmap.put(Channel.FR, FRCoordinates);
            sampleMap.put(Channel.FR, FRCoordinates);
            System.out.println("Front Right coordinates are: " + Double.toString(FRx) + ", " + Double.toString(FRy));
            
        }
        else if (event.getSource()==BL) {
            Bounds BLBounds = BL.localToScene(BL.getBoundsInLocal());
            Bounds headBounds = head.localToScene(head.getBoundsInLocal());
            double BLx = ((BLBounds.getMinX() - headBounds.getMinX())/(-35));
            double BLy = ((BLBounds.getMinY() - headBounds.getMinY())/(50));
            BLCoordinates[0] = (float)BLx;
            BLCoordinates[1] = (float)BLy;
            BLCoordinates[2] = 0;
            hashmap.put(Channel.BL, BLCoordinates);
            sampleMap.put(Channel.BL, BLCoordinates);
            System.out.println("Back Left coordinates are: " + Double.toString(BLx) + ", " + Double.toString(BLy));
            
        }
        else if (event.getSource()==BR) {
            Bounds BRBounds = BR.localToScene(BR.getBoundsInLocal());
            Bounds headBounds = head.localToScene(head.getBoundsInLocal());
            double BRx = ((BRBounds.getMinX() - headBounds.getMinX())/(50));
            double BRy = ((BRBounds.getMinY() - headBounds.getMinY())/(-50));
            BRCoordinates[0] = (float)BRx;
            BRCoordinates[1] = (float)BRy;
            BRCoordinates[2] = 0;
            hashmap.put(Channel.BR, BRCoordinates);
            sampleMap.put(Channel.BR, BRCoordinates);
            System.out.println("Back Right coordinates are: " + Double.toString(BRx) + ", " + Double.toString(BRy));
            
        }

        if (playSample) callDSVSCA("x1.wav", HRTFPath, 256, sampleMap, true);
    }
    
    private void initializeHashmap() {
        try {
            
            FLCoordinates[0] = 1;
            FLCoordinates[1] = 1;
            FLCoordinates[2] = 0;
            
            FCCoordinates[0] = 1;
            FCCoordinates[1] = 0;
            FCCoordinates[2] = 0;
            
            FRCoordinates[0] = 1;
            FRCoordinates[1] = -1;
            FRCoordinates[2] = 0;
            
            BLCoordinates[0] = -1;
            BLCoordinates[1] = 1;
            BLCoordinates[2] = 0;
            
            BRCoordinates[0] = -1;
            BRCoordinates[1] = -1;
            BRCoordinates[2] = 0;
            
            hashmap.put(Channel.FL, FLCoordinates);
            hashmap.put(Channel.FC, FCCoordinates);
            hashmap.put(Channel.FR, FRCoordinates);
            hashmap.put(Channel.BL, BLCoordinates);
            hashmap.put(Channel.BR, BRCoordinates);
            
            /*
            Bounds FLBounds = FL.localToScene(FL.getBoundsInLocal());
            Bounds FRBounds = FR.localToScene(FR.getBoundsInLocal());
            Bounds FCBounds = FC.localToScene(FC.getBoundsInLocal());
            Bounds BLBounds = BL.localToScene(BL.getBoundsInLocal());
            Bounds BRBounds = BR.localToScene(BR.getBoundsInLocal());
            Bounds headBounds = head.localToScene(head.getBoundsInLocal());

            double FLx = FLBounds.getMinX() - headBounds.getMinX();
            double FLy = FLBounds.getMinY() - headBounds.getMinY();
            double FRx = FRBounds.getMinX() - headBounds.getMinX();
            double FRy = FRBounds.getMinY() - headBounds.getMinY();
            double FCx = FCBounds.getMinX() - headBounds.getMinX();
            double FCy = FCBounds.getMinY() - headBounds.getMinY();
            double BLx = BLBounds.getMinX() - headBounds.getMinX();
            double BLy = BLBounds.getMinY() - headBounds.getMinY();
            double BRx = BRBounds.getMinX() - headBounds.getMinX();
            double BRy = BRBounds.getMinY() - headBounds.getMinY();
            
            System.out.println("Front Left coordinates are: " + Double.toString(FLx) + ", " + Double.toString(FLy));
            System.out.println("Front Right coordinates are: " + Double.toString(FRx) + ", " + Double.toString(FRy));
            System.out.println("Front Center coordinates are: " + Double.toString(FCx) + ", " + Double.toString(FCy));
            System.out.println("Back Left coordinates are: " + Double.toString(BLx) + ", " + Double.toString(BLy));
            System.out.println("Back Right coordinates are: " + Double.toString(BRx) + ", " + Double.toString(BRy));
            */
        }
        
        catch (NullPointerException e){
            
        }
        

    }
    
    public void playSound(String fileName) {
        File file = new File(fileName);
        try {
            final Clip clip = (Clip)AudioSystem.getLine(new Line.Info(Clip.class));
            clip.addLineListener((LineEvent event) -> {
                if (event.getType() == LineEvent.Type.STOP) clip.close();
            });
            
            clip.open(AudioSystem.getAudioInputStream(file));
            clip.start();
            System.out.println("Play Sound: " + fileName);
        }
        catch (Exception e) {
            System.out.println(e.getMessage());
        }
    }
    
    private void callDSVSCA(String videoFile, String sofaFile, int blockSize, HashMap<Channel, float[]> coordinates, Boolean isSample) {
        ArrayList<String> cmd = new ArrayList<String>();
        cmd.add("./DSVSCA");
        if (isSample) cmd.add("--sample");
        cmd.add("--video=" + videoFile);
        cmd.add("--sofa=" + sofaFile);
        cmd.add("--block-size=" + blockSize);
        
        Channel firstChannel = Channel.FL;
        for (Entry<Channel, float[]> coord : coordinates.entrySet()) {
            firstChannel = coord.getKey();
            cmd.add(String.format("--%s=%f,%f,%f", coord.getKey().toString().toLowerCase(), coord.getValue()[0], coord.getValue()[1], coord.getValue()[2]));
        }
        
        for (int i = 0; i < cmd.size(); i++) System.out.print(cmd.get(i) + " ");
        System.out.println();

        Process proc;
        try {
            proc = Runtime.getRuntime().exec(cmd.toArray(new String[0]));
        } catch (IOException e) {
            System.out.println("Error when executing process: " + String.join(" ", cmd));
            return;
        }

        BufferedReader consoleOutput = new BufferedReader(new InputStreamReader(proc.getInputStream()));

        String line;
        try {
            Boolean startedProgress = false;
            // readLine works with my \r only update to the screen since Java considers it to be a new-line
            while ((line = consoleOutput.readLine()) != null) {
            	if (line.startsWith("Progress:")) {
                    if (!startedProgress) startedProgress = true;
                    progressText.setText(line);
                    System.out.print(line + "\r");
                }
                else {
                    if (startedProgress) System.out.println();
                    System.out.println(line);
                }
            }
            System.out.println();
        } catch (IOException e) {
            System.out.println("Error while reading console output.");
        }
        
        try {
            consoleOutput.close();
        } catch (IOException e) {}
        
        if (isSample) {
            String fileName = videoFile.substring(0, videoFile.lastIndexOf('.'));

            if (null != firstChannel) switch (firstChannel) {
                case FL:
                    fileName += "-fl";
                    break;
                case FC:
                    fileName += "-fc";
                    break;
                case FR:
                    fileName += "-fr";
                    break;
                case BL:
                    fileName += "-bl";
                    break;
                case BR:
                    fileName += "-br";
                    break;
                default:
                    break;
            }
            fileName += videoFile.substring(videoFile.lastIndexOf('.'));
            playSound(fileName);
        }
    }
    
    @FXML
    private void setTextField(ActionEvent event) {
        if (event.getSource()==setInputPathButton) {
            System.out.println("You set the input path to"+" \""+inputPathTextField.getText()+"\"");
            inputPathLabel.setText(inputPathTextField.getText());
            inPath = inputPathTextField.getText();
        }
        else if (event.getSource()==setOutputPathButton) {
            System.out.println("You set the output path to"+" \""+outputPathTextField.getText()+"\"");
            outputPathLabel.setText(outputPathTextField.getText());
            outPath = outputPathTextField.getText();
        }
        else if (event.getSource()==setHRTFPathButton) {
            System.out.println("You set the HRTF path to"+" \""+HRTFPathTextField.getText()+"\"");
            HRTFPathLabel.setText(HRTFPathTextField.getText());
            HRTFPath = HRTFPathTextField.getText();
        }
    }
    
    @FXML
    private void switchScene(ActionEvent event) throws IOException{
        Stage stage = null; 
        Parent root = null;
        if(event.getSource()==calibrateButton){
            stage=(Stage) calibrateButton.getScene().getWindow();
            root = FXMLLoader.load(getClass().getResource("calibrateScene.fxml"));
            System.out.println("Going to the Calibrate Scene...");
            initializeHashmap();
        }
        else if(event.getSource()==backButton){
            stage=(Stage) backButton.getScene().getWindow();
            root = FXMLLoader.load(getClass().getResource("FXMLDocument.fxml"));
            System.out.println("Going back to the Home Scene...");
        }
        else if(event.getSource()==homeButton) {
            stage=(Stage) homeButton.getScene().getWindow();
            root = FXMLLoader.load(getClass().getResource("FXMLDocument.fxml"));
            System.out.println("Going back to the Home Scene...");
        }
        else if(event.getSource()==convertButton){
            stage=(Stage) convertButton.getScene().getWindow();
            root = FXMLLoader.load(getClass().getResource("completeScene.fxml"));
            System.out.println("Going to the Complete Scene...");
        }
        else if (event.getSource()==startButton) {
            stage=(Stage) startButton.getScene().getWindow();
            root = FXMLLoader.load(getClass().getResource("completeScene.fxml"));
            System.out.println("Going to the Complete Scene...");
        }
        Scene scene = new Scene(root);
        stage.setScene(scene);
        stage.show();
    }
    
    @FXML
    public void drag(MouseEvent t) {
        orgSceneX = t.getSceneX();
        orgSceneY = t.getSceneY();
        orgTranslateX = ((Circle)(t.getSource())).getTranslateX();
        orgTranslateY = ((Circle)(t.getSource())).getTranslateY();
        ((Circle)(t.getSource())).setFill(Color.rgb(64, 255, 118));
    }
    
    
    @FXML
    public void drop(MouseEvent t) {
        double offsetX = t.getSceneX() - orgSceneX;
        double offsetY = t.getSceneY() - orgSceneY;
        double newTranslateX = orgTranslateX + offsetX;
        double newTranslateY = orgTranslateY + offsetY;
             
        ((Circle)(t.getSource())).setTranslateX(newTranslateX);
        ((Circle)(t.getSource())).setTranslateY(newTranslateY);
        
        updateHashmap(t, true);
        ((Circle)(t.getSource())).setFill(Color.rgb(11, 31, 50));
    }
    
    @FXML
    public void playTest(ActionEvent event) {
        if (thread != null) {
            try {
                thread.join();
            } catch (InterruptedException ex) {}
        }
        
        thread = new Thread(() -> {
            HashMap<Channel, float[]> sampleMap = new HashMap<Channel, float[]>();
            
            for (Entry<Channel, float[]> coord : hashmap.entrySet()) {
                switch (coord.getKey()) {
                    case FL:
                        FL.setFill(Color.rgb(64, 255, 118));
                        break;
                    case FC:
                        FC.setFill(Color.rgb(64, 255, 118));
                        break;
                    case FR:
                        FR.setFill(Color.rgb(64, 255, 118));
                        break;
                    case BL:
                        BL.setFill(Color.rgb(64, 255, 118));
                        break;
                    case BR:
                        BR.setFill(Color.rgb(64, 255, 118));
                        break;
                    default:
                        return;
                }
                
                sampleMap.put(coord.getKey(), coord.getValue());
                callDSVSCA("x1.wav", HRTFPath, 256, sampleMap, true);
                sampleMap.clear();
                
                switch (coord.getKey()) {
                    case FL:
                        FL.setFill(Color.rgb(11, 31, 50));
                        break;
                    case FC:
                        FC.setFill(Color.rgb(11, 31, 50));
                        break;
                    case FR:
                        FR.setFill(Color.rgb(11, 31, 50));
                        break;
                    case BL:
                        BL.setFill(Color.rgb(11, 31, 50));
                        break;
                    case BR:
                        BR.setFill(Color.rgb(11, 31, 50));
                        break;
                    default:
                        return;
                }
            }
        });
        
        thread.start();
    }
    
    @FXML
    private void start(ActionEvent event) {
        if (thread != null) {
            try {
                thread.join();
            } catch (InterruptedException ex) {}
        }
        
        thread = new Thread(() -> {
            try {
                progressText.setVisible(true);
                callDSVSCA(inPath, HRTFPath, 256, hashmap, false);
            }
            catch (Exception ex) {

            }

            Platform.runLater(() -> {
                try {
                    switchScene(event);
                }
                catch (IOException e) {}
            });
        });
        
        thread.start();
    }    
    
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        String urlString = url.toString();
        int index = urlString.lastIndexOf('/');
        if (index < 0) index = urlString.lastIndexOf('\\');

        String fxmlFile = url.toString().substring(index + 1);
        if ("FXMLDocument.fxml".equals(fxmlFile)) inputPathLabel.setText(inPath);
        else if ("calibrateScene.fxml".equals(fxmlFile)) HRTFPathLabel.setText(HRTFPath);
    }    
    
}
