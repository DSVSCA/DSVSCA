/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package dsvsca;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map.Entry;
import java.util.ResourceBundle;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.geometry.Bounds;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.AnchorPane;
import javafx.scene.shape.Circle;
import javafx.stage.Stage;

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
    private String inPath;
    private String outPath;
    private String HRTFPath;
    double orgSceneX, orgSceneY, orgTranslateX, orgTranslateY;
    public enum Channel {
        FL,
        FC,
        FR,
        BL,
        BR,
        LFE
    }
    HashMap<Channel, int[]> hashmap = new HashMap<Channel, int[]>();
    int[] FLCoordinates = new int[3];
    int[] FCCoordinates = new int[3];
    int[] FRCoordinates = new int[3];
    int[] BLCoordinates = new int[3];
    int[] BRCoordinates = new int[3];
    
    @FXML
    private void updateHashmap(MouseEvent event) {
        if (event.getSource()==FL) {
            Bounds FLBounds = FL.localToScene(FL.getBoundsInLocal());
            Bounds headBounds = head.localToScene(head.getBoundsInLocal());
            double FLx = ((FLBounds.getMinX() - headBounds.getMinX())/(-35));
            double FLy = ((FLBounds.getMinY() - headBounds.getMinY())/(-43));
            FLCoordinates[0] = (int) FLx;
            FLCoordinates[1] = (int) FLy;
            hashmap.put(Channel.FL, FLCoordinates);
            System.out.println("Front Left coordinates are: " + Double.toString(FLx) + ", " + Double.toString(FLy));
            
        }
        else if (event.getSource()==FC) {
            Bounds FCBounds = FC.localToScene(FC.getBoundsInLocal());
            Bounds headBounds = head.localToScene(head.getBoundsInLocal());
            double FCx = ((FCBounds.getMinX() - headBounds.getMinX())/(8));
            double FCy = ((FCBounds.getMinY() - headBounds.getMinY())/(-43));
            FCCoordinates[0] = (int) FCx;
            FCCoordinates[1] = (int) FCy;
            hashmap.put(Channel.FC, FCCoordinates);
            System.out.println("Front Center coordinates are: " + Double.toString(FCx) + ", " + Double.toString(FCy));
            
        }
        else if (event.getSource()==FR) {
            Bounds FRBounds = FR.localToScene(FR.getBoundsInLocal());
            Bounds headBounds = head.localToScene(head.getBoundsInLocal());
            double FRx = ((FRBounds.getMinX() - headBounds.getMinX())/(50));
            double FRy = ((FRBounds.getMinY() - headBounds.getMinY())/(-43));
            FRCoordinates[0] = (int) FRx;
            FRCoordinates[1] = (int) FRy;
            hashmap.put(Channel.FR, FRCoordinates);
            System.out.println("Front Right coordinates are: " + Double.toString(FRx) + ", " + Double.toString(FRy));
            
        }
        else if (event.getSource()==BL) {
            Bounds BLBounds = BL.localToScene(BL.getBoundsInLocal());
            Bounds headBounds = head.localToScene(head.getBoundsInLocal());
            double BLx = ((BLBounds.getMinX() - headBounds.getMinX())/(-35));
            double BLy = ((BLBounds.getMinY() - headBounds.getMinY())/(50));
            BLCoordinates[0] = (int) BLx;
            BLCoordinates[1] = (int) BLy;
            hashmap.put(Channel.BL, BLCoordinates);
            System.out.println("Back Left coordinates are: " + Double.toString(BLx) + ", " + Double.toString(BLy));
            
        }
        else if (event.getSource()==BR) {
            Bounds BRBounds = BR.localToScene(BR.getBoundsInLocal());
            Bounds headBounds = head.localToScene(head.getBoundsInLocal());
            double BRx = ((BRBounds.getMinX() - headBounds.getMinX())/(50));
            double BRy = ((BRBounds.getMinY() - headBounds.getMinY())/(50));
            BRCoordinates[0] = (int) BRx;
            BRCoordinates[1] = (int) BRy;
            hashmap.put(Channel.BR, BRCoordinates);
            System.out.println("Back Right coordinates are: " + Double.toString(BRx) + ", " + Double.toString(BRy));
            
        }
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
    
    private void callDSVSCA(String videoFile, String sofaFile, int blockSize, HashMap<Channel, int[]> coordinates) {
        ArrayList<String> cmd = new ArrayList<String>();
        cmd.add("./DSVSCA");
        cmd.add("--video=" + videoFile);
        cmd.add("--sofa=" + sofaFile);
        cmd.add("--block-size=" + blockSize);
        
        for (Entry<Channel, int[]> coord : coordinates.entrySet()) {
            cmd.add(String.format("--%s=%d,%d,%d", coord.getKey().toString().toLowerCase(), coord.getValue()[0], coord.getValue()[1], coord.getValue()[2]));
        }

        Process proc;
	try {
            proc = Runtime.getRuntime().exec((String[])cmd.toArray());
	} catch (IOException e) {
            System.out.println("Error when executing process: " + String.join(" ", cmd));
            return;
	}

        BufferedReader consoleOutput = new BufferedReader(new InputStreamReader(proc.getInputStream()));

        String line;
        try {
            // readLine works with my \r only update to the screen since Java considers it to be a new-line
            while ((line = consoleOutput.readLine()) != null) {
		System.out.println(line);
            }
	} catch (IOException e) {
            System.out.println("Error while reading console output.");
	}
        
        try {
            consoleOutput.close();
	} catch (IOException e) {}
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
    }
    
    
    @FXML
    public void drop(MouseEvent t) {
        double offsetX = t.getSceneX() - orgSceneX;
        double offsetY = t.getSceneY() - orgSceneY;
        double newTranslateX = orgTranslateX + offsetX;
        double newTranslateY = orgTranslateY + offsetY;
             
        ((Circle)(t.getSource())).setTranslateX(newTranslateX);
        ((Circle)(t.getSource())).setTranslateY(newTranslateY);
        
        updateHashmap(t);
    }
    
    @FXML
    private void start(ActionEvent event) {
        try {
            callDSVSCA(inPath, HRTFPath, 256, hashmap);
        }
        catch (Exception ex) {
            
        }
        
        try {
            switchScene(event);
        }
        catch (IOException e) {
            
        }
    }    
    
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        // TODO
    }    
    
}
