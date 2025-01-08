import tkinter as tk
from tkinter import ttk
from tkinter import filedialog
from tkinter import messagebox

import numpy as np

from PIL import Image, ImageTk

import ctypes
import time
import os

class MyApp(tk.Tk):
    def __init__(self):
        super().__init__()

        # configure window
        self.title("Laplace Filter - ASM vs C")
        self.geometry("1200x600")
        self.minsize(720, 480)

        # configure grid
        self.columnconfigure(0, weight=0) # settings column
        self.columnconfigure(1, weight=2, uniform="image columns") # image before column
        self.columnconfigure(2, weight=2, uniform="image columns") # image after column
        self.rowconfigure(0, weight=1)

        # create frames
        self.create_settings_frame()
        self.create_image_before_frame()
        self.create_image_after_frame()

        # resize event handling
        self.resize_event_id = None
        self.bind("<Configure>", self.on_resize)

        # load DLLs
        try:
            self.init_dlls()
        except Exception as e:
            messagebox.showerror("DLL error", str(e))


    def on_resize(self, event):
        if event.width == self.winfo_width() and event.height == self.winfo_height():
            return

        if self.resize_event_id is not None:
            self.after_cancel(self.resize_event_id)

        self.resize_event_id = self.after(100, self.on_resize_end)
    
    def on_resize_end(self):
        self.show_thumbnails()

    def open_input_image(self):
        # open file dialog
        file_path = filedialog.askopenfilename(title="Select the image file", filetypes=[("Image files", "*.jpg *.png *.bmp")])
        if not file_path:
            return

        # load image
        self.img_before = Image.open(file_path)

        # show thumbnails
        self.show_thumbnails()

        # update image description
        width, height = self.img_before.size
        img_desc = f"File name: {file_path.split('/')[-1]}\nDimensions: {width}x{height} ({width*height} pixels)"
        self.img_before_description.config(text=img_desc)

        # enable process button
        self.process_button.config(state=tk.NORMAL)
    
    def show_thumbnails(self):
        # determine thumbnail size
        frame_width = self.panel_image_before.winfo_width() - 20
        frame_height = self.panel_image_before.winfo_height() - 20
        thumbnail_size = (frame_width, frame_height)

        # show "before" thumbnail
        if self.img_before is not None:
            thumbnail_before = self.img_before.copy()
            thumbnail_before.thumbnail(thumbnail_size)
            self.tk_img_before = ImageTk.PhotoImage(thumbnail_before)
            self.label_img_before.config(image=self.tk_img_before)

        # show "after" thumbnail
        if self.img_after is not None:
            thumbnail_after = self.img_after.copy()
            thumbnail_after.thumbnail(thumbnail_size)
            self.tk_img_after = ImageTk.PhotoImage(thumbnail_after)
            self.label_img_after.config(image=self.tk_img_after)

    def init_dlls(self):
        # load DLLs
        this_file_path = os.path.dirname(os.path.abspath(__file__))
        c_file_path = os.path.join(this_file_path, "libraries", "laplace_c.dll")
        asm_file_path = os.path.join(this_file_path, "libraries", "laplace_asm.dll")
        self.c_dll = ctypes.CDLL(c_file_path)
        self.asm_dll = ctypes.CDLL(asm_file_path)

        # extract functions 
        self.c_laplace = self.c_dll.laplace
        self.asm_laplace = self.asm_dll.laplace

        # set argtypes and restype
        argtypes = [ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_ubyte), ctypes.POINTER(ctypes.c_ubyte), ctypes.c_int, ctypes.c_int]
        self.c_laplace.argtypes = argtypes
        self.c_laplace.restype = None
        self.asm_laplace.argtypes = argtypes
        self.asm_laplace.restype = None
    
        # dlls loaded successfully, enable file selection
        self.file_button.config(state=tk.NORMAL)

    def create_settings_frame(self):
        # Containing frame
        self.panel_settings = ttk.Frame(self, borderwidth=2, relief="groove", padding=5, width=200)
        self.panel_settings.grid(row=0, column=0, sticky="nsew", padx=5, pady=5)

        # Header
        panel_header = ttk.Label(self.panel_settings, text="Settings", font=("Arial", 12))
        panel_header.pack(pady=(5,30), anchor=tk.W)

        # File selection
        self.file_button = ttk.Button(self.panel_settings, text="Pick image file", command=self.open_input_image, state=tk.DISABLED)
        self.file_button.pack(pady=(5,30), anchor=tk.W, fill=tk.X)

        # Number of threads
        thread_label = ttk.Label(self.panel_settings, text="Number of threads:")
        thread_label.pack(pady=5, anchor=tk.W)

        self.thread_entry = ttk.Spinbox(self.panel_settings, from_=1, to=64)
        self.thread_entry.set(1)
        self.thread_entry.pack(pady=(5,30), anchor=tk.W)

        # Implementation
        implementation_label = ttk.Label(self.panel_settings, text="Implementation:")
        implementation_label.pack(pady=5, anchor=tk.W)

        self.implementation_var = tk.StringVar()
        self.implementation_var.set("C")

        c_radio = ttk.Radiobutton(self.panel_settings, text="C", variable=self.implementation_var, value="C")
        c_radio.pack(pady=(5,2), anchor=tk.W)

        asm_radio = ttk.Radiobutton(self.panel_settings, text="ASM", variable=self.implementation_var, value="ASM")
        asm_radio.pack(pady=(2,30), anchor=tk.W)

        # Amplification
        amplification_label = ttk.Label(self.panel_settings, text="Amplification:")
        amplification_label.pack(pady=5, anchor=tk.W)

        self.amplification_entry = ttk.Spinbox(self.panel_settings, from_=1, to=64)
        self.amplification_entry.set(8)
        self.amplification_entry.pack(pady=5, anchor=tk.W)

        # Process button
        self.process_button = ttk.Button(self.panel_settings, text="Process image", command=self.process_image, state=tk.DISABLED)
        self.process_button.pack(pady=5, anchor=tk.W, side=tk.BOTTOM, fill=tk.X)

    def create_image_before_frame(self):
        # Containing frame
        self.panel_image_before = ttk.Frame(self, borderwidth=2, relief="groove", padding=5)
        self.panel_image_before.grid(row=0, column=1, sticky="nsew", padx=5, pady=5)

        # Bottom frame
        self.panel_image_before_description = ttk.Frame(self.panel_image_before)
        self.panel_image_before_description.pack(pady=5, side=tk.BOTTOM, fill=tk.X)

        # Input image data
        self.img_before: Image = None
        self.tk_img_before: ImageTk.PhotoImage = None

        # Image-displaying label
        self.label_img_before = ttk.Label(self.panel_image_before)
        self.label_img_before.pack(pady=5)

        # Image description header
        self.img_before_header = ttk.Label(self.panel_image_before_description, text="Input file info: ", font=("Arial", 12))
        self.img_before_header.pack(pady=5, anchor=tk.W)

        # Image description
        self.img_before_description = ttk.Label(self.panel_image_before_description, text="[No file selected]")
        self.img_before_description.pack(pady=5, anchor=tk.W)
    
    def create_image_after_frame(self):
        # Containing frame
        self.panel_image_after = ttk.Frame(self, borderwidth=2, relief="groove", padding=5)
        self.panel_image_after.grid(row=0, column=2, sticky="nsew", padx=5, pady=5)

        # Bottom frame
        self.panel_image_after_description = ttk.Frame(self.panel_image_after)
        self.panel_image_after_description.pack(pady=5, side=tk.BOTTOM, fill=tk.X)

        # Output image data
        self.img_after: Image = None
        self.tk_img_after: ImageTk.PhotoImage = None

        # Image-displaying label
        self.label_img_after = ttk.Label(self.panel_image_after)
        self.label_img_after.pack(pady=5)

        # Proccessing description header
        self.img_after_info_header = ttk.Label(self.panel_image_after_description, text="Image processing info: ", font=("Arial", 12))
        self.img_after_info_header.pack(pady=5, anchor=tk.W)

        # Processing description
        self.img_after_description = ttk.Label(self.panel_image_after_description, text="[No image processed]")
        self.img_after_description.pack(pady=5, anchor=tk.W)

        # "View" button
        view_button = ttk.Button(self.panel_image_after_description, text="View full image", command=self.show_full_image)
        view_button.pack(pady=5, anchor=tk.W)
    
    def show_full_image(self):
        if self.img_after is not None:
            self.img_after.show()
    
    def process_image(self):
        # extract image data into an np.array
        width, height = self.img_before.size
        self.img_before = self.img_before.convert("RGB") # ensure 4 channels
        img_data = np.array(self.img_before, dtype=np.uint8).flatten()

        # prepare input and output ctypes-arrays
        input_array = (ctypes.c_ubyte * len(img_data)).from_buffer(img_data)
        output_array = (ctypes.c_ubyte * len(img_data))()

        # determine call parameters and function
        num_threads = int(self.thread_entry.get())
        amplification = int(self.amplification_entry.get())
        func = self.c_laplace if self.implementation_var.get() == "C" else self.asm_laplace
        
        # process image while measuring time
        start_time = time.perf_counter()
        func(width, height, input_array, output_array, num_threads, amplification)
        end_time = time.perf_counter()

        # update processing description
        execution_time_us = int((end_time - start_time) * 1_000_000)
        per_thread_time = execution_time_us // num_threads
        proc_desc = f"Processing time: {execution_time_us} μs\nTime per thread: {per_thread_time} μs"
        self.img_after_description.config(text=proc_desc)

        # convert output array to image
        output_data = np.frombuffer(output_array, dtype=np.uint8).reshape((height, width, 3))
        self.img_after = Image.fromarray(output_data, "RGB")
        
        # show thumbnails
        self.show_thumbnails()

if __name__ == "__main__":
    app = MyApp()
    app.mainloop()
