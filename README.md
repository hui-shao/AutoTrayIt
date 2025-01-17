## Auto Tray It

Automatically hide windows containing specific keywords. 

language: **EN** | [中文](./README_CN.md)

### Usage

1. Download the pre-built binary from the `Release` page, or compile from source.

2. Create a configuration file `config.toml` and place it in the same directory as the binary.

   The program supports multiple keywords and `config.example.toml` in the repository is available for reference.

3. Double-click the `exe` file, the main program will run and minimize to the tray in the lower right corner by default. 

   At the same time, all windows containing preset keywords will be hidden, and each hidden window will have its corresponding icon in the tray.

4. By double-clicking these icons, you can switch the window visibility between "show/hide".

5. When the hidden window is closed, the corresponding tray icon is automatically deleted (with a certain delay).

6. When the main program exits, all hidden windows will be restored to visible and all related tray icons will be deleted.

7. To manually remove a tray icon: Hold down Ctrl and double-click the icon. The corresponding window will be visible again and will no longer be automatically hidden.

### Other matters

+ Currently, the main window of this program has no content and is blank. This is normal.
+ You can add this program to the startup program to automatically hide some windows.

