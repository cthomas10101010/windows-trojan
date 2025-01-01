from fastapi import FastAPI, File, UploadFile, Request, HTTPException
from fastapi.responses import JSONResponse
from fastapi.middleware.cors import CORSMiddleware
import os
import logging
from datetime import datetime
import uuid

# Initialize FastAPI app
app = FastAPI()

# Configure CORS (Optional: Adjust origins as needed)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Allow all origins; modify as per your security requirements
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Define the directory to save uploaded files
UPLOAD_DIRECTORY = "uploads"

# Ensure the upload directory exists
os.makedirs(UPLOAD_DIRECTORY, exist_ok=True)

# Configure Logging
logger = logging.getLogger("uvicorn.error")
handler = logging.StreamHandler()
formatter = logging.Formatter(
    "%(asctime)s - %(levelname)s - %(message)s"
)
handler.setFormatter(formatter)
logger.addHandler(handler)
logger.setLevel(logging.INFO)

@app.post("/upload")
async def upload_file(request: Request):
    """
    Endpoint to handle file uploads via POST requests.
    Supports both multipart/form-data and application/octet-stream.

    For multipart/form-data:
        - Expects a 'file' field.

    For application/octet-stream:
        - If 'X-Filename' header is provided, uses it as the filename.
        - Otherwise, assigns a unique filename.

    Args:
        request (Request): The incoming request object.

    Returns:
        JSONResponse: A JSON response confirming the file save location or an error.
    """
    client_host = request.client.host if request.client else "Unknown"
    timestamp = datetime.utcnow().isoformat()
    content_type = request.headers.get('Content-Type', '')

    logger.info(f"Received upload request from {client_host} at {timestamp} with Content-Type: {content_type}")

    try:
        if 'multipart/form-data' in content_type:
            # Handle multipart/form-data
            form = await request.form()
            if 'file' not in form:
                logger.error("No 'file' field in multipart/form-data.")
                raise HTTPException(status_code=400, detail="No 'file' field in form data.")
            file: UploadFile = form['file']
            filename = file.filename or f"uploaded_{uuid.uuid4()}"
            file_location = os.path.join(UPLOAD_DIRECTORY, filename)
            logger.info(f"Processing multipart upload for file: {filename}")

            with open(file_location, "wb") as f:
                content = await file.read()
                f.write(content)
            logger.info(f"Successfully saved file: {file_location}")
            return JSONResponse(content={"message": f"File '{filename}' saved at '{file_location}'"})

        elif 'application/octet-stream' in content_type:
            # Handle raw binary data
            filename = request.headers.get('X-Filename')
            if not filename:
                # Assign a unique filename if X-Filename is missing
                filename = f"uploaded_{uuid.uuid4()}"
                logger.warning("Missing 'X-Filename' header for raw upload. Assigning unique filename.")

            file_location = os.path.join(UPLOAD_DIRECTORY, filename)
            logger.info(f"Processing raw upload for file: {filename}")

            body = await request.body()
            if not body:
                logger.error("No data received in the body for raw upload.")
                raise HTTPException(status_code=400, detail="No data received in the body.")

            with open(file_location, "wb") as f:
                f.write(body)
            logger.info(f"Successfully saved file: {file_location}")
            return JSONResponse(content={"message": f"File '{filename}' saved at '{file_location}'"})

        else:
            logger.error(f"Unsupported Content-Type: {content_type}")
            raise HTTPException(status_code=415, detail=f"Unsupported Content-Type: {content_type}")

    except HTTPException as http_exc:
        logger.error(f"HTTP Exception: {http_exc.detail}")
        raise http_exc
    except Exception as e:
        logger.error(f"Error processing upload: {str(e)}")
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/")
async def read_root():
    """
    Endpoint to display a welcome message at the root path.

    Returns:
        dict: A welcome message with instructions.
    """
    return {
        "message": "Welcome to the file upload server.",
        "instructions": "Use POST /upload to upload files.",
        "upload_methods": {
            "multipart/form-data": "Send a 'file' field with the file.",
            "application/octet-stream": "Send raw binary data with 'X-Filename' header specifying the filename."
        }
    }

# Middleware for detailed logging
@app.middleware("http")
async def log_requests(request: Request, call_next):
    logger.info(f"Incoming request: {request.method} {request.url}")
    response = await call_next(request)
    logger.info(f"Response status: {response.status_code} for {request.method} {request.url}")
    return response


##uvicorn app:app --host 0.0.0.0 --port 80