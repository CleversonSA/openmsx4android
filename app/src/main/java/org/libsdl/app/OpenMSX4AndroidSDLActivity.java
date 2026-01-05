package org.libsdl.app;

import android.content.Context;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.FrameLayout;
import android.widget.ImageButton;

import org.libsdl.app.SDLActivity;
import org.libsdl.app.SDLSurface;

public class OpenMSX4AndroidSDLActivity extends SDLActivity {

    private View mFocusView; // idealmente a SDLSurface

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // SDLActivity cria a view do SDL internamente.
        // A gente vai "achar" a SDLSurface e colocar um overlay por cima.
        final ViewGroup content = findViewById(android.R.id.content);

        // Tenta localizar a SDLSurface dentro da hierarchy
        mFocusView = findSdlsurface(content);
        if (mFocusView == null) {
            // fallback: usa o decorView
            mFocusView = getWindow().getDecorView();
        }

        // Cria um overlay com botão flutuante
        FrameLayout overlay = new FrameLayout(this);
        overlay.setLayoutParams(new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT));

        ImageButton kb = new ImageButton(this);
        kb.setImageResource(android.R.drawable.ic_menu_edit); // ícone simples
        kb.setBackgroundResource(android.R.drawable.btn_default); // básico

        int size = dp(48);
        int margin = dp(12);

        FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(size, size);
        lp.gravity = Gravity.BOTTOM | Gravity.END;
        lp.setMargins(margin, margin, margin, margin);
        kb.setLayoutParams(lp);

        kb.setOnClickListener(v -> showKeyboard());

        overlay.addView(kb);

        // Adiciona o overlay por cima do conteúdo existente
        content.addView(overlay);
    }

    private void showKeyboard() {
        runOnUiThread(() -> {
            // 1) Peça para o SDL "entrar em modo texto" do lado nativo (ver JNI abaixo)
            nativeRequestTextInput(true);

            // 2) Foque no EditText escondido do SDL, se existir
            View target = null;

            try {
                // SDLActivity.mTextEdit é protected static no SDLActivity (subclasse tem acesso)
                if (mTextEdit != null) {
                    target = mTextEdit;
                }
            } catch (Throwable ignored) {}

            if (target == null) {
                // fallback: se por algum motivo não existir ainda, usa a surface (menos ideal)
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

    // JNI (implementa no C/C++)
    private static native void nativeRequestTextInput(boolean enable);

    // (Opcional) um botão para esconder também
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