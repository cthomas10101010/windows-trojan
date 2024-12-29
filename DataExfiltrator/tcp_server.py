from fastapi import FastAPI, File, UploadFile
from fastapi.responses import JSONResponse
import os

# Initialize FastAPI app
app = FastAPI()

# Define the directory to save uploaded files
UPLOAD_DIRECTORY = "uploads"

# Ensure the upload directory exists
os.makedirs(UPLOAD_DIRECTORY, exist_ok=True)

@app.post("/")
async def upload_file(file: UploadFile = File(...)):
    """
    Endpoint to handle file uploads via POST requests.

    Args:
        file (UploadFile): The uploaded file object.

    Returns:
        JSONResponse: A JSON response confirming the file save location.
    """
    file_location = os.path.join(UPLOAD_DIRECTORY, file.filename)
    try:
        with open(file_location, "wb") as f:
            f.write(await file.read())
        return JSONResponse(content={"message": f"File {file.filename} saved at {file_location}"})
    except Exception as e:
        return JSONResponse(content={"error": str(e)}, status_code=500)

@app.get("/")
async def read_root():
    """
    Endpoint to display a welcome message at the root path.

    Returns:
        dict: A welcome message with instructions.
    """
    return {"message": "Welcome to the file upload server. Use POST to upload files."}