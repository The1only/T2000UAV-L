package com.wit.example.UI;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import java.util.List;

public class CustomAdapter extends ArrayAdapter<ListItem> {

    public CustomAdapter(Context context, List<ListItem> items) {
        super(context, 0, items);
    }

    private void status(String str) {
        System.out.println(str);
    }


    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        // Get the data item for this position
        ListItem item = getItem(position);

        // Populate the data into the template view using the data object
        status(item.getTitle());
        status(item.getData());

        // Return the completed view to render on screen
        return convertView;
    }
}
