import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map.Entry;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextPane;
import javax.swing.SwingUtilities;
import javax.swing.border.EmptyBorder;

public class DSVSCA extends JFrame {
    public enum Channel {
        FL,
        FC,
        FR,
        BL,
        BR,
        LFE
    }

	private static final long serialVersionUID = 1L;

	public DSVSCA() {

        initUI();
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

    private void initUI() {

        JPanel panel = new JPanel();
        panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

        panel.setBorder(new EmptyBorder(new Insets(40, 60, 40, 60)));

        
        JPanel textPanel = new JPanel(new BorderLayout());
        textPanel.setBorder(BorderFactory.createEmptyBorder(10, 20, 10, 20));
        JTextPane pane = new JTextPane();

        pane.setContentType("text/html");
        String fileName = "<p>File Name</p>";
        pane.setText(fileName);
        pane.setEditable(false);
        textPanel.add(pane);
        
        
        JPanel textPanel2 = new JPanel(new BorderLayout());
        textPanel2.setBorder(BorderFactory.createEmptyBorder(10, 20, 10, 20));
        JTextPane pane2 = new JTextPane();

        pane2.setContentType("text/html");
        String outputLocation = "<p>Output Location</p>";
        pane2.setText(outputLocation);
        pane2.setEditable(false);
        textPanel2.add(pane2);
        
        JButton fileButton = new JButton("Select File");
        
        fileButton.addActionListener(new ActionListener() {
        	public void actionPerformed(ActionEvent e) {
                JOptionPane.showMessageDialog(null, "File Button was Clicked!");
            }
        });
        
        JButton locationButton = new JButton("Select Output Location");
        
        locationButton.addActionListener(new ActionListener() {
        	public void actionPerformed(ActionEvent e) {
                JOptionPane.showMessageDialog(null, "Location Button was Clicked!");
            }
        });
        
        JButton convertButton = new JButton("Convert");
        
        convertButton.addActionListener(new ActionListener() {
        	public void actionPerformed(ActionEvent e) {
                JOptionPane.showMessageDialog(null, "Convert Button was Clicked!");
            }
        });
        
        JButton calibrateButton = new JButton("Calibrate");
        
        calibrateButton.addActionListener(new ActionListener() {
        	public void actionPerformed(ActionEvent e) {
                JOptionPane.showMessageDialog(null, "Calibrate Button was Clicked!");
            }
        });
        
        panel.add(fileButton);
        
        panel.add(textPanel);
        
        panel.add(Box.createRigidArea(new Dimension(0, 5)));
        panel.add(locationButton);
        panel.add(textPanel2);
        panel.add(Box.createRigidArea(new Dimension(0, 5)));
        panel.add(convertButton);
        panel.add(Box.createRigidArea(new Dimension(0, 5)));
        panel.add(calibrateButton);

        add(panel);

        pack();

        setTitle("DSVSCA");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocationRelativeTo(null);
    }

    public static void main(String[] args) {

        SwingUtilities.invokeLater(() -> {
        	DSVSCA ex = new DSVSCA();
            ex.setVisible(true);
        });
    }
}
