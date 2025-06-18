
// PCBcircleDetectView.cpp : implementation of the CPCBcircleDetectView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "PCBcircleDetect.h"
#endif

#include "PCBcircleDetectDoc.h"
#include "PCBcircleDetectView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPCBcircleDetectView

IMPLEMENT_DYNCREATE(CPCBcircleDetectView, CView)

BEGIN_MESSAGE_MAP(CPCBcircleDetectView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CPCBcircleDetectView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_FILE_UPLOADIMAGE, &CPCBcircleDetectView::OnFileUploadimage)
	ON_COMMAND(ID_OPERATIONS_DISTBETCENTERS, &CPCBcircleDetectView::OnCalculateCircleDistances)
END_MESSAGE_MAP()

// CPCBcircleDetectView construction/destruction

CPCBcircleDetectView::CPCBcircleDetectView() noexcept
{
	// TODO: add construction code here

}

CPCBcircleDetectView::~CPCBcircleDetectView()
{
}

BOOL CPCBcircleDetectView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CPCBcircleDetectView drawing

void CPCBcircleDetectView::OnDraw(CDC* pDC)
{
	CPCBcircleDetectDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here


	// Display the image with mouse interaction
	if (!m_image.empty()) {
		// Display the image with OpenCV to MFC CDC
		DisplayImage(pDC, m_image);
	}
	else {
		pDC->TextOutW(10, 10, _T("Upload an image using the File menu."));
	}

	// Draw the points (Green dots) on the main window
	for (const auto& point : userClickedPoints) {
		cv::circle(m_image, point, 5, cv::Scalar(0, 255, 0), -1); // Draw green dot at clicked points
	}

	// After 5 points, process and check if it's a circle
	if (userClickedPoints.size() >= 5) {
		ProcessUserDefinedShape(m_image);
	}

}


// CPCBcircleDetectView printing


void CPCBcircleDetectView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CPCBcircleDetectView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CPCBcircleDetectView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CPCBcircleDetectView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CPCBcircleDetectView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CPCBcircleDetectView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CPCBcircleDetectView diagnostics

#ifdef _DEBUG
void CPCBcircleDetectView::AssertValid() const
{
	CView::AssertValid();
}

void CPCBcircleDetectView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CPCBcircleDetectDoc* CPCBcircleDetectView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPCBcircleDetectDoc)));
	return (CPCBcircleDetectDoc*)m_pDocument;
}
#endif //_DEBUG


// CPCBcircleDetectView message handlers


void CPCBcircleDetectView::OnFileUploadimage()
{
	// TODO: Add your command handler code here

	// Open file dialog to select an image
	CFileDialog fileDlg(TRUE, _T("Image Files"), nullptr, OFN_FILEMUSTEXIST,
		_T("Image Files (*.bmp;*.jpg;*.jpeg;*.png)|*.bmp;*.jpg;*.jpeg;*.png|All Files (*.*)|*.*||"));

	if (fileDlg.DoModal() == IDOK)
	{
		m_imagePath = fileDlg.GetPathName(); // Get the selected file path
		CT2A asciiPath(m_imagePath); // Convert to ASCII for OpenCV
		m_image = cv::imread(asciiPath.m_psz, cv::IMREAD_COLOR); // Load the image

		if (m_image.empty())
		{
			AfxMessageBox(_T("Failed to load the image."));
		}
		else
		{
			// Store the original image for reference
			m_displayImage = m_image.clone();

			// Create an OpenCV window for the image
			cv::namedWindow("Image Window", cv::WINDOW_NORMAL);

			// Show the image in that window
			cv::imshow("Image Window", m_image);
			// Set up the mouse callback
			cv::setMouseCallback("Image Window", OnMouseCallback, this);
			Invalidate(); // Force redraw of window
		}
	}
}


void CPCBcircleDetectView::DisplayImage(CDC* pDC, const cv::Mat& image)
{
	// Get the client rectangle of the window
	CRect rect;
	GetClientRect(&rect);

	// Calculate the aspect ratio of the image
	double imageAspectRatio = static_cast<double>(image.cols) / image.rows;
	double windowAspectRatio = static_cast<double>(rect.Width()) / rect.Height();

	int displayWidth, displayHeight;
	int offsetX = 0, offsetY = 0;

	// Check if the image is wider than the window or vice versa and adjust accordingly
	if (imageAspectRatio > windowAspectRatio)
	{
		// Image is wider than the window
		displayWidth = rect.Width();
		displayHeight = static_cast<int>(rect.Width() / imageAspectRatio);
		offsetY = (rect.Height() - displayHeight) / 2; // Center vertically
	}
	else
	{
		// Image is taller than the window
		displayHeight = rect.Height();
		displayWidth = static_cast<int>(rect.Height() * imageAspectRatio);
		offsetX = (rect.Width() - displayWidth) / 2; // Center horizontally
	}

	// Resize the image to fit the window while maintaining the aspect ratio
	cv::Mat resizedImage;
	cv::resize(image, resizedImage, cv::Size(displayWidth, displayHeight));

	// Convert OpenCV image (BGR) to Bitmap (BGRA with alpha channel)
	cv::Mat temp;
	cv::cvtColor(resizedImage, temp, cv::COLOR_BGR2BGRA);

	BITMAPINFO bitmapInfo;
	memset(&bitmapInfo, 0, sizeof(BITMAPINFO));
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biWidth = temp.cols;
	bitmapInfo.bmiHeader.biHeight = -temp.rows; // Negative for top-down image
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	// Draw the resized image at the center of the window
	StretchDIBits(pDC->m_hDC, offsetX, offsetY, displayWidth, displayHeight,
		0, 0, temp.cols, temp.rows,
		temp.data, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

	
}






bool CPCBcircleDetectView::IsCircle(const std::vector<cv::Point>& points)
{
	cv::Point2f center;
	float radius;

	// Fit a circle around the points
	cv::minEnclosingCircle(points, center, radius);

	// Check if all points lie approximately on the circle's boundary
	float threshold = 10.0f; // Allowable distance from the circle
	for (const auto& point : points)
	{
		float distance = cv::norm(cv::Point2f(point) - center);
		if (std::abs(distance - radius) > threshold)
		{
			return false; // At least one point deviates too much
		}
	}
	return true;
}


 void CPCBcircleDetectView::OnMouseCallback(int event, int x, int y, int flags, void* userdata)
{
	// TODO: Add your implementation code here.
	
	 CPCBcircleDetectView* self = reinterpret_cast<CPCBcircleDetectView*>(userdata);
	 if (event == cv::EVENT_LBUTTONDOWN) // Left-click to select a point
	 {
		 // Store the clicked point
		 self->userClickedPoints.emplace_back(x, y);

		 //// Draw a small circle at the clicked point
		 cv::circle(self->m_image, cv::Point(x, y), 5, cv::Scalar(0, 255, 0), -1);

		
		 // Redraw the image immediately (this will trigger OnDraw)
		 cv::circle(self->m_image, cv::Point(x, y), 5, cv::Scalar(0, 255, 0), -1);

		 self->Invalidate();

		 cv::imshow("Image Window", self->m_image);


		 // If enough points are clicked, process the shape
		 if (self->userClickedPoints.size() >= 5) // Minimum points to define a shape
		 {
			 self->ProcessUserDefinedShape(self->m_image);
		 }
		
	 }

 }



void CPCBcircleDetectView::ProcessUserDefinedShape(cv::Mat& image)
{
	// TODO: Add your implementation code here.
	if (userClickedPoints.size() < 5)
	{
		AfxMessageBox(_T("Please click at least 5 points to define a shape."));
		return;
	}


	// Check if the points form a circle
	if (IsCircle(userClickedPoints))
	{
		cv::Scalar circleColor(0, 0, 255); // Red for circle

		// Draw the approximated circle
		cv::Point2f center;
		float radius;
		cv::minEnclosingCircle(userClickedPoints, center, radius);

		// Store the detected circle's center
		detectedCircleCenters.push_back(center);


		cv::circle(image, center, static_cast<int>(radius), circleColor, 2);

		

		// Draw the diameter as a dotted line
		int diameter = static_cast<int>(2 * radius);
		cv::Point start(center.x - radius, center.y);
		cv::Point end(center.x + radius, center.y);

		// Draw the dotted line
		for (int i = 0; i < diameter; i += 5)
		{
			// Draw a small line segment for the dotted effect
			cv::Point p1(start.x + i, start.y);
			cv::Point p2(start.x + i + 3, start.y);
			cv::line(image, p1, p2, cv::Scalar(255, 0, 0), 2);
		}

		cv::Scalar coordColor(255, 0, 0); // Blue color for coordinates
		std::string coordText = "(" + std::to_string(int(center.x)) + ", " + std::to_string(int(center.y)) + ")";
		
		// Draw the coordinates at the center of the circle
		cv::putText(image, coordText, cv::Point(center.x + 10, center.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, coordColor, 2);


		// Draw the center of the circle with a red dot
		cv::Scalar centerDotColor(0, 0, 255); // Red color for center
		int centerDotRadius = 5; // Radius for the red dot
		cv::circle(image, center, centerDotRadius, centerDotColor, -1); // Filled red circle at center



		AfxMessageBox(_T("Shape detected: Circle"));

	

	}
	else
	{
		AfxMessageBox(_T("Shape detected: Not a circle"));
	}

	// Refresh the window to show the updated image
	//cv::imshow("Shape Detection", image);

	// Reset points for the next shape detection
	userClickedPoints.clear();
	Invalidate();
}



void CPCBcircleDetectView::OnCalculateCircleDistances()
{
	// Call the function to calculate the distance between detected circle centers
	CalculateDistanceBetweenCenters();
}



void CPCBcircleDetectView::CalculateDistanceBetweenCenters()
{
	// Ensure there are at least two circles detected
	if (detectedCircleCenters.size() < 2)
	{
		AfxMessageBox(_T("Need at least two circles to calculate distance."));
		return;
	}

	// Iterate over each pair of detected circles and calculate distance
	for (size_t i = 0; i < detectedCircleCenters.size() - 1; ++i)  // Loop through all but the last circle
	{
		for (size_t j = i + 1; j < detectedCircleCenters.size(); ++j)  // Compare circle i with all following circles
		{
			// Calculate Euclidean distance between the two centers (only different circles)
			double distance = cv::norm(detectedCircleCenters[i] - detectedCircleCenters[j]);

			// Skip if distance is too small (avoid drawing a line for same center)
			if (distance < 1.0) {
				continue;  // This prevents drawing lines for centers that are very close
			}

			// Draw a blue line between the centers of circle i and circle j
			cv::line(m_image, detectedCircleCenters[i], detectedCircleCenters[j], CV_RGB(0, 0, 255), 2);  // 2 is line thickness

			// Optionally, draw the distance as text near the center of the line
			cv::Point midPoint = (detectedCircleCenters[i] + detectedCircleCenters[j]) / 2;
			std::string distanceText = "Dist: " + std::to_string(distance);
			cv::putText(m_image, distanceText, midPoint, cv::FONT_HERSHEY_SIMPLEX, 0.6, CV_RGB(0, 0, 255), 1);  // Draw text in red

		}
	}
	Invalidate();
}



