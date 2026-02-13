package org.libsdl.app;

import android.content.Context;
import android.os.Bundle;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.FrameLayout;
import android.widget.ImageButton;

import com.openmsx.openmsx4android.OpenMSXPathUtil;


/**
 * Java's OpenMSX SDL x ANativeWindow layer and integration
 *
 * @Author Cleverson SA - 2026
 *
 */
public class OpenMSX4AndroidSDLActivity extends SDLActivity {

    private View mFocusView;

    private FrameLayout dpadPanel;
    private FrameLayout dextraPanel;

    private static final int DPAD_UP = 0;
    private static final int DPAD_DOWN = 1;
    private static final int DPAD_LEFT = 2;
    private static final int DPAD_RIGHT = 3;
    private static final int DPAD_ENTER = 4;
    private static final int DPAD_SPACE = 5;
    private static final int DPAD_TAB = 6;
    private static final int DPAD_ESC = 7;

    static {
        System.loadLibrary("main"); // ajuste para o nome real do seu .so
    }

    private static native void nativeSetSharePath(String path);


    /**
     * Create the Android's virtual keyboard arrow keys overlay
     * @return FrameLayout
     */
    protected FrameLayout createVirtualArrowKeys(FrameLayout overlay) {
        ImageButton kb = new ImageButton(this);
        kb.setImageResource(android.R.drawable.ic_menu_directions);
        kb.setBackgroundResource(android.R.drawable.btn_default);

        int size = dp(72);
        int margin = dp(12);
        int spacing = dp(12);

        FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(size, size);
        lp.gravity = Gravity.BOTTOM | Gravity.END;
        lp.setMargins(margin, margin, margin + size+spacing, margin);
        kb.setLayoutParams(lp);

        kb.setOnClickListener(v -> {
            toggleDpad();
            toggleDextra();
        });

        overlay.addView(kb);

        return overlay;
    }

    /**
     * Create the Android's virtual keyboard button overlay
     * @return FrameLayout
     */
    protected FrameLayout createVirtualKeyboardButton(FrameLayout overlay) {


        ImageButton kb = new ImageButton(this);
        kb.setImageResource(android.R.drawable.ic_menu_edit); // ícone simples
        kb.setBackgroundResource(android.R.drawable.btn_default); // básico

        int size = dp(72);
        int margin = dp(12);

        FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(size, size);
        lp.gravity = Gravity.BOTTOM | Gravity.END;
        lp.setMargins(margin, margin, margin, margin);
        kb.setLayoutParams(lp);

        kb.setOnClickListener(v -> {
            showKeyboard();
        });

        overlay.addView(kb);

        return overlay;
    }

    /**
     * Create the directional panel
     * @param overlay
     * @return
     */
    protected FrameLayout createDirectionPanel(FrameLayout overlay) {

        dpadPanel = buildDpadPanel();
        dpadPanel.setVisibility(View.GONE);
        overlay.addView(dpadPanel);

        dextraPanel = buildExtraPanel();
        dextraPanel.setVisibility(View.GONE);
        overlay.addView(dextraPanel);

        return overlay;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        String sharePath = OpenMSXPathUtil.getOpenmsxSharePath(this);
        nativeSetSharePath(sharePath);

        // SDLActivity internal view and SDLSurface overlay association
        final ViewGroup content = findViewById(android.R.id.content);

        mFocusView = findSdlsurface(content);
        if (mFocusView == null) {
            mFocusView = getWindow().getDecorView();
        }

        FrameLayout overlay = new FrameLayout(this);
        overlay.setLayoutParams(new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT));

        overlay = createVirtualArrowKeys(overlay);
        overlay = createVirtualKeyboardButton(overlay);
        overlay = createDirectionPanel(overlay);

        content.addView(overlay);
    }

    private void toggleDpad() {
        runOnUiThread(() -> {
            if (dpadPanel == null) return;

            if (dpadPanel.getVisibility() == View.VISIBLE) {
                dpadPanel.setVisibility(View.GONE);
                releaseAllDpad(); // Avoid hold state of a key
            } else {
                dpadPanel.setVisibility(View.VISIBLE);
            }
        });
    }

    private void toggleDextra() {
        runOnUiThread(() -> {
            if (dextraPanel == null) return;

            if (dextraPanel.getVisibility() == View.VISIBLE) {
                dextraPanel.setVisibility(View.GONE);
                releaseAllDpad(); // Avoid hold state of a key
            } else {
                dextraPanel.setVisibility(View.VISIBLE);
            }
        });
    }

    private void releaseAllDpad() {
        nativeSendDpad(DPAD_UP, false);
        nativeSendDpad(DPAD_DOWN, false);
        nativeSendDpad(DPAD_LEFT, false);
        nativeSendDpad(DPAD_RIGHT, false);
        nativeSendDpad(DPAD_ENTER, false);
        nativeSendDpad(DPAD_SPACE, false);
        nativeSendDpad(DPAD_TAB, false);
        nativeSendDpad(DPAD_ESC, false);
    }

    private View.OnTouchListener dpadTouch(int dir) {
        return (v, event) -> {
            switch (event.getActionMasked()) {
                case android.view.MotionEvent.ACTION_DOWN:
                    nativeSendDpad(dir, true);
                    v.performHapticFeedback(android.view.HapticFeedbackConstants.KEYBOARD_TAP);
                    return true;

                case android.view.MotionEvent.ACTION_UP:
                case android.view.MotionEvent.ACTION_CANCEL:
                    nativeSendDpad(dir, false);
                    return true;
            }
            return false;
        };
    }

    /**
     * Build the direction panel
     * @return
     */
    private FrameLayout buildDpadPanel() {
        FrameLayout panel = new FrameLayout(this);

        int btn = dp(72);
        int pad = dp(12);
        int box = btn * 3 + pad * 2;

        FrameLayout.LayoutParams plp = new FrameLayout.LayoutParams(box, box);
        plp.gravity = android.view.Gravity.BOTTOM | android.view.Gravity.END;

        int margin = dp(8);
        int pencilSize = dp(72);
        plp.setMargins(margin, margin, margin, margin + pencilSize + dp(15));
        panel.setLayoutParams(plp);

        android.widget.Button enter = new android.widget.Button(this);
        enter.setText("Enter");
        enter.setOnTouchListener(dpadTouch(DPAD_ENTER));
        FrameLayout.LayoutParams enterLp = new FrameLayout.LayoutParams(btn, btn, Gravity.CENTER_VERTICAL | Gravity.CENTER_HORIZONTAL);
        panel.addView(enter, enterLp);

        android.widget.Button up = new android.widget.Button(this);
        up.setText("↑");
        up.setOnTouchListener(dpadTouch(DPAD_UP));
        FrameLayout.LayoutParams upLp = new FrameLayout.LayoutParams(btn, btn, android.view.Gravity.TOP | android.view.Gravity.CENTER_HORIZONTAL);
        panel.addView(up, upLp);

        android.widget.Button down = new android.widget.Button(this);
        down.setText("↓");
        down.setOnTouchListener(dpadTouch(DPAD_DOWN));
        FrameLayout.LayoutParams downLp = new FrameLayout.LayoutParams(btn, btn, android.view.Gravity.BOTTOM | android.view.Gravity.CENTER_HORIZONTAL);
        panel.addView(down, downLp);

        android.widget.Button left = new android.widget.Button(this);
        left.setText("←");
        left.setOnTouchListener(dpadTouch(DPAD_LEFT));
        FrameLayout.LayoutParams leftLp = new FrameLayout.LayoutParams(btn, btn, android.view.Gravity.CENTER_VERTICAL | android.view.Gravity.START);
        panel.addView(left, leftLp);

        android.widget.Button right = new android.widget.Button(this);
        right.setText("→");
        right.setOnTouchListener(dpadTouch(DPAD_RIGHT));
        FrameLayout.LayoutParams rightLp = new FrameLayout.LayoutParams(btn, btn, android.view.Gravity.CENTER_VERTICAL | android.view.Gravity.END);
        panel.addView(right, rightLp);

        return panel;
    }

    /**
     * Build the EXTRA panel
     * @return
     */
    private FrameLayout buildExtraPanel() {
        FrameLayout panel = new FrameLayout(this);

        int btn = dp(72);
        int pad = dp(12);
        int box = btn * 3 + pad * 2;

        FrameLayout.LayoutParams plp = new FrameLayout.LayoutParams(box, box);
        plp.gravity = android.view.Gravity.BOTTOM | android.view.Gravity.START;

        int margin = dp(8);
        int pencilSize = dp(72);
        plp.setMargins(margin, margin, margin, margin + pencilSize + dp(10));
        panel.setLayoutParams(plp);

        android.widget.Button esc = new android.widget.Button(this);
        esc.setText("ESC");
        esc.setOnTouchListener(dpadTouch(DPAD_ESC));
        FrameLayout.LayoutParams escLp = new FrameLayout.LayoutParams(btn, btn, Gravity.TOP | android.view.Gravity.START);
        panel.addView(esc, escLp);

        android.widget.Button tab = new android.widget.Button(this);
        tab.setText("TAB");
        tab.setOnTouchListener(dpadTouch(DPAD_TAB));
        FrameLayout.LayoutParams tabLp = new FrameLayout.LayoutParams(btn, btn, Gravity.CENTER_VERTICAL | android.view.Gravity.START);
        panel.addView(tab, tabLp);

        android.widget.Button space = new android.widget.Button(this);
        space.setText("Space");
        space.setOnTouchListener(dpadTouch(DPAD_SPACE));
        FrameLayout.LayoutParams spaceLp = new FrameLayout.LayoutParams(btn, btn, android.view.Gravity.BOTTOM | android.view.Gravity.START);
        panel.addView(space, spaceLp);

        return panel;
    }

    private void showKeyboard() {
        runOnUiThread(() -> {
            // JNI Virtual keyboard activation (C++)
            nativeRequestTextInput(true);

            View target = null;

            try {
                if (mTextEdit != null) {
                    target = mTextEdit;
                }
            } catch (Throwable ignored) {}

            if (target == null) {
                // If does not exist the internal text edit, use surface instead
                target = mFocusView;
            }

            if (target != null) {
                target.setFocusable(true);
                target.setFocusableInTouchMode(true);
                target.requestFocus();

                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                if (imm != null) {
                    imm.showSoftInput(target, InputMethodManager.SHOW_IMPLICIT);
                }
            }
        });
    }

    /**
     * JNI Implementation to activate and deactivate the Virtual keyboard
     * @param enable
     */
    private static native void nativeRequestTextInput(boolean enable);

    /**
     * JNI implementation to send soft keyboard virtual arrow keys
     * @param dir
     * @param isDown
     */
    private static native void nativeSendDpad(int dir, boolean isDown);

    /**
     * Hides the Virtual Keyboard
     */
    private void hideKeyboard() {
        runOnUiThread(() -> {
            InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
            if (imm != null && mFocusView != null) {
                imm.hideSoftInputFromWindow(mFocusView.getWindowToken(), 0);
            }
        });
    }

    private int dp(int v) {
        return Math.round(v * getResources().getDisplayMetrics().density);
    }

    /**
     * Finds the active SDL Surface
     * @param root
     * @return
     */
    private View findSdlsurface(ViewGroup root) {
        for (int i = 0; i < root.getChildCount(); i++) {
            View c = root.getChildAt(i);
            if (c instanceof SDLSurface) return c;
            if (c instanceof ViewGroup) {
                View r = findSdlsurface((ViewGroup) c);
                if (r != null) return r;
            }
        }
        return null;
    }

}