import tkinter as tk
from tkinter import ttk
from tkinter import filedialog
from tkinter import messagebox

import numpy as np

from PIL import Image, ImageTk

import ctypes

class MyApp(tk.Tk):
    def __init__(self):
        super().__init__()

        self.input_file_path: str = ""

        self.title("Laplace Filter - ASM vs C")
        self.geometry("800x600")

        self.create_main_panel()
        self.create_settings_panel()
        self.create_image_before_panel()
        self.create_image_after_panel()

        try:
            self.init_dlls()
        except Exception as e:
            messagebox.showerror("DLL error", str(e))


    def open_input_image(self):
        file_path = filedialog.askopenfilename(title="Select the image file", filetypes=[("Image files", "*.jpg *.png *.bmp")])
        self.img_before = Image.open(file_path)
        self.file_label.config(text=file_path.split("/")[-1])
        self.process_button.config(state=tk.NORMAL)
        self.show_before_thumbnail()
    
    def show_before_thumbnail(self):
        thumbnail = self.img_before.copy()
        thumbnail.thumbnail((300,300))
        self.tk_img_before = ImageTk.PhotoImage(thumbnail)
        self.label_img_before.config(image=self.tk_img_before)
    
    def init_dlls(self):
        # load DLLs
        self.c_dll = ctypes.CDLL("../x64/Release/laplace_c.dll")
        self.asm_dll = ctypes.CDLL("../x64/Release/laplace_asm.dll")

        if self.c_dll is None:
            raise Exception("Failed to load laplace_c.dll")
        if self.asm_dll is None:
            raise Exception("Failed to load laplace_asm.dll")

        # extract functions 
        self.c_laplace = self.c_dll.laplace
        self.asm_laplace = self.asm_dll.laplace

        if self.c_laplace is None:
            raise Exception("Failed to extract laplace function from laplace_c.dll")
        if self.asm_laplace is None:
            raise Exception("Failed to extract laplace function from laplace_asm.dll")

        # set argtypes and restype
        argtypes = [ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_ubyte), ctypes.POINTER(ctypes.c_ubyte), ctypes.c_int, ctypes.c_int]
        self.c_laplace.argtypes = argtypes
        self.c_laplace.restype = None
        self.asm_laplace.argtypes = argtypes
        self.asm_laplace.restype = None

    
    def create_main_panel(self):
        self.panel_main = ttk.Frame(self)
        self.panel_main.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

    def create_settings_panel(self):
        self.panel_settings = ttk.Frame(self.panel_main, borderwidth=2, relief="groove", padding=10)
        self.panel_settings.pack(side=tk.LEFT, fill=tk.Y, padx=5)

        # Header
        panel_header = ttk.Label(self.panel_settings, text="Settings", font=("Arial", 12))
        panel_header.pack(pady=(5,30), anchor=tk.W)


        # File selection
        file_button = ttk.Button(self.panel_settings, text="Pick image file", command=self.open_input_image)
        file_button.pack(pady=5, anchor=tk.W)

        self.file_label = ttk.Label(self.panel_settings, text="[No file selected]")
        self.file_label.pack(pady=5, anchor=tk.W)


        # Number of threads
        thread_label = ttk.Label(self.panel_settings, text="Number of threads:")
        thread_label.pack(pady=5, anchor=tk.W)

        self.thread_entry = ttk.Spinbox(self.panel_settings, from_=1, to=64)
        self.thread_entry.set(1)
        self.thread_entry.pack(pady=5, anchor=tk.W)


        # Implementation
        implementation_label = ttk.Label(self.panel_settings, text="Implementation:")
        implementation_label.pack(pady=5, anchor=tk.W)

        self.implementation_var = tk.StringVar()

        c_radio = ttk.Radiobutton(self.panel_settings, text="C", variable=self.implementation_var, value="C")
        c_radio.pack(pady=5, anchor=tk.W)

        asm_radio = ttk.Radiobutton(self.panel_settings, text="ASM", variable=self.implementation_var, value="ASM")
        asm_radio.pack(pady=5, anchor=tk.W)

        self.implementation_var.set("C")


        # Amplification
        amplification_label = ttk.Label(self.panel_settings, text="Amplification:")
        amplification_label.pack(pady=5, anchor=tk.W)

        self.amplification_entry = ttk.Spinbox(self.panel_settings, from_=1, to=64)
        self.amplification_entry.set(1)
        self.amplification_entry.pack(pady=5, anchor=tk.W)


        # Process button
        self.process_button = ttk.Button(self.panel_settings, text="Process image", command=self.process_image, state=tk.DISABLED)
        self.process_button.pack(pady=5, anchor=tk.W, side=tk.BOTTOM)

    def create_image_before_panel(self):
        self.panel_image_before = ttk.Frame(self.panel_main, borderwidth=2, relief="groove", padding=10)
        self.panel_image_before.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=5)

        # Init Image-related data
        self.img_before: Image = None
        self.tk_img_before: ImageTk.PhotoImage = None

        # Add widgets to image_before panel
        self.label_img_before = ttk.Label(self.panel_image_before)
        self.label_img_before.pack(pady=5)
    
    def create_image_after_panel(self):
        self.panel_image_after = ttk.Frame(self.panel_main, borderwidth=2, relief="groove", padding=10)
        self.panel_image_after.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=5)

        # Init Image-related data
        self.img_after: Image = None
        self.tk_img_after: ImageTk.PhotoImage = None

        # Add widgets to image_after panel
        self.label_img_after = ttk.Label(self.panel_image_after)
        self.label_img_after.pack(pady=5)
    
    def process_image(self):
        width, height = self.img_before.size
        self.img_before = self.img_before.convert("RGB") # ensure 4 channels
        img_data = np.array(self.img_before, dtype=np.uint8).flatten()

        input_array = (ctypes.c_ubyte * len(img_data)).from_buffer(img_data)
        output_array = (ctypes.c_ubyte * len(img_data))()

        self.asm_laplace(width, height, input_array, output_array, 1, 16)

        output_data = np.frombuffer(output_array, dtype=np.uint8).reshape((height, width, 3))
        output_img = Image.fromarray(output_data, "RGB")

        output_img.show()



# Run the application
if __name__ == "__main__":
    app = MyApp()
    app.mainloop()
