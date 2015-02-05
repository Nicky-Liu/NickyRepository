package com.example.studyffmpeg;

import com.study.Activity.videoActivity;

import android.R.integer;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends Activity {
	private TextView tView;
	private Button btnstart;

	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		DisplayMetrics dm = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(dm);
		Log.i("width = ",  dm.widthPixels +"");//宽度
		Log.i("height = ",  dm.heightPixels +"");//高度
		
		getWidget();
		initEnv();
	}

	private void getWidget() {
		tView = (TextView) findViewById(R.id.textViewTest);
		btnstart = (Button)findViewById(R.id.buttontest);
	}
	
	private void initEnv(){
		btnstart.setText("开始");
		btnstart.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View arg0) {
				// TODO Auto-generated method stub
				Intent intent = new Intent();
				intent.setClass(MainActivity.this,
						videoActivity.class);
				startActivity(intent);
			}
		});
	}
}
