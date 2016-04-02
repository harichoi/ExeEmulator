package kr.selfcontrol.exeemulator;

import java.io.File;
import java.io.FileNotFoundException;

import java.util.ArrayList;
import java.util.List;

import android.app.AlertDialog;

import android.app.AlertDialog.Builder;

import android.app.ListActivity;
import android.content.Context;

import android.os.Bundle;

import android.text.Layout;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.LayoutInflater;

import android.view.MotionEvent;

import android.view.View;

import android.view.View.OnTouchListener;

import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;

import android.widget.ListView;

import android.widget.ScrollView;
import android.widget.TextView;


public class MainActivity extends ListActivity {
    static {
        System.loadLibrary("dasm");
    }

    private String filename, dirPath;
    private TextView myPath, empty;
    private String root = "/sdcard/";
    private List<String> item = null;
    private List<String> path = null;
    private TextView resultView;
    private ScrollView scrollView;
    private Button enterButton;
    private EditText editText;
    // private ImageView bmImage;
    //private BitmapFactory.Options bmOptions;
    private File[] files;
    private ListView lv;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        myPath = (TextView) findViewById(R.id.path);
        empty = (TextView) findViewById(R.id.empty);
        lv = (ListView) findViewById(android.R.id.list);

        getDir(root);

        empty.setText("Entities : " + files.length);
        lv.setOnTouchListener(new OnTouchListener() {
            public boolean onTouch(View v, MotionEvent e) {
                empty.setText("Entities : " + files.length);
                return false;
            }
        });
        dasm dd=new dasm();
        dd.callBackTest();
    }

    private void getDir(String dirPath) {
        myPath.setText("Location : Root" + dirPath);
        this.dirPath = dirPath;

        item = new ArrayList<String>();
        path = new ArrayList<String>();

        File f = new File(dirPath);
        files = f.listFiles();
        if (!dirPath.equals(root)) {
            item.add(root);
            item.add("../");
            path.add(root);
            path.add(f.getParent());
        }

        for (int i = 0; i < files.length; i++) {
            File file = files[i];
            path.add(file.getPath());
            if (file.isDirectory()) {
                item.add("[" + file.getName() + "]");
            } else if (file.isFile()) {
                item.add(file.getName());
            }
        }
        setListAdapter(new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, item));
    }


    public void onListItemClick(ListView parent, View v, int position, long id) {
        File file = new File(path.get(position));
        if (file.isDirectory()) {
            if (file.canRead()) {
                getDir(path.get(position));
                empty.setText("Entities : " + files.length);
            } else {
                empty.setText("Entities : " + files.length + "  Not opened");
            }
        } else {
            filename = file.getName();
            String ext = filename.substring(filename.lastIndexOf('.') + 1, filename.length());
            if (ext.equalsIgnoreCase("exe")) {
                if (!ext.equalsIgnoreCase("exe"))
                    empty.setText("Entities : " + files.length + "   Can't read atrribute!");
                try {
                    showExeDialog();
                } catch (FileNotFoundException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                } catch (Exception exc) {
                    exc.printStackTrace();
                }
            } else {
                empty.setText("Entities : " + files.length + "  It's not exe file!");
            }
        }
    }


    public void executeNextCode(){
        Log.d("executeNExtCode","exeucte");
        int num=0;
        while(dasm.isFinish()==0){
            num++;
            if (dasm.getInputType() == 1) {
                if(editText.getText().toString().trim().isEmpty()) break;
                dasm.inputChar(editText.getText().toString());
                editText.setText("");
            } else if (dasm.getInputType() == 2) {
                if(editText.getText().toString().trim().isEmpty()) break;
                dasm.inputFloat((float) Float.parseFloat(editText.getText().toString()));
                editText.setText("");
            } else if (dasm.getInputType() == 3) {
                if(editText.getText().toString().trim().isEmpty()) break;
                dasm.inputInt((int) Integer.parseInt(editText.getText().toString()));
                editText.setText("");
            } else {
                //Log.d("hahas", dasm.readNext());
                dasm.readNext();
                if (dasm.getPrintBuffer() != null) {
                    String buffer = dasm.getPrintBuffer();
                    resultView.append(buffer);
                    Log.d("buffer", buffer);
                    dasm.printBufferClear();
              //      break;
                }
            }
            if(num>500) {
                break;
            }
        }
        if(dasm.isFinish()==1){
            editText.setText("Finished!");
            editText.setEnabled(false);
            enterButton.setText("Finished");
            enterButton.setEnabled(false);
        }
        if(dasm.getInputType()!=0){
            editText.setText("");
            editText.setEnabled(true);
            enterButton.setText("Enter");
        } else {
            editText.setText("");
            editText.setEnabled(false);
            enterButton.setText("Next");
        }
        scrollView.fullScroll(View.FOCUS_DOWN);
    }
    private void showExeDialog() throws FileNotFoundException {
        Builder ad;
        final Context mContext = MainActivity.this;
        LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(LAYOUT_INFLATER_SERVICE);
        final View textEntryView = inflater.inflate(R.layout.view_exe, null);

        resultView = (TextView) textEntryView.findViewById(R.id.tv_result);


        editText=(EditText)textEntryView.findViewById(R.id.et_str);
        scrollView=(ScrollView)textEntryView.findViewById(R.id.scrollView1);
        enterButton=(Button)textEntryView.findViewById(R.id.bt_enter);
        enterButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                executeNextCode();
            }
        });

        resultView.setMovementMethod(new ScrollingMovementMethod());
        resultView.setText("");

        ad = new AlertDialog.Builder(this);
        ad.setTitle(filename);

        dasm.fileLoad(dirPath + "/" + filename);

        executeNextCode();

        ad.setView(textEntryView);
        ad.setNeutralButton("Close", null);

        ad.show();

    }

}

