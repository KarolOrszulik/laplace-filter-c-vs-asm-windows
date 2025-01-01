import tkinter as tk
from tkinter import ttk
from tkinter import filedialog

from PIL import Image, ImageTk

import ctypes
import os

class MyApp(tk.Tk):
    def __init__(self):
        super().__init__()

        self.file_path: str = ""

        self.title("Laplace Filter - ASM vs C")
        self.geometry("800x600")

        # Create frames
        panel_main = ttk.Frame(self)
        panel_main.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        panel_settings = ttk.Frame(panel_main, borderwidth=2, relief="groove", padding=10)
        panel_settings.pack(side=tk.LEFT, fill=tk.Y, padx=5)

        panel_image_before = ttk.Frame(panel_main, borderwidth=2, relief="groove", padding=10)
        panel_image_before.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=5)

        panel_image_after = ttk.Frame(panel_main, borderwidth=2, relief="groove", padding=10)
        panel_image_after.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=5)

        # Add widgets to panel1
        self.file_button = ttk.Button(panel_settings, text="Pick image file", command=self.open_file)
        self.file_button.pack(pady=5, anchor=tk.W)

        self.file_label = ttk.Label(panel_settings, text="[No file selected]", font=("Arial", 10))
        self.file_label.pack(pady=5, anchor=tk.W)

        # Widgets of Image Before
        self.img_before: Image = None
        self.tk_img_before: ImageTk.PhotoImage = None

        self.label_img_before = ttk.Label(panel_image_before)
        self.label_img_before.pack(pady=5)

    def open_file(self):
        file_path = filedialog.askopenfilename(title="Select the image file", filetypes=[("Image files", "*.jpg *.png *.bmp")])
        self.file_path = file_path
        self.file_label.config(text=file_path.split("/")[-1])
        self.set_image(file_path)
    
    def set_image(self, image_path: str):
        self.img_before = Image.open(image_path)
        self.img_before.thumbnail((300,300))
        self.tk_img_before = ImageTk.PhotoImage(self.img_before)
        self.label_img_before.config(image=self.tk_img_before)
    
    def init_dlls(self):
        self.c_dll = ctypes.CDLL("../x64/Release/laplace_c.dll")
        self.asm_dll = ctypes.CDLL("../x64/Release/laplace_asm.dll")

        self.c_laplace = self.c_dll.laplace
        self.asm_laplace = self.asm_dll.laplace

# Run the application
if __name__ == "__main__":
    app = MyApp()
    app.mainloop()
