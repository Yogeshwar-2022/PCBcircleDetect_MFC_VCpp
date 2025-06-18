
// PCBcircleDetectView.h : interface of the CPCBcircleDetectView class
//

#pragma once
#include <atlimage.h> 
#include "opencv2/opencv.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
class CPCBcircleDetectView : public CView
{
protected: // create from serialization only
	CPCBcircleDetectView() noexcept;
	DECLARE_DYNCREATE(CPCBcircleDetectView)

// Attributes
public:
	CPCBcircleDetectDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CPCBcircleDetectView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	// Add this in the private section of the class declaration
	std::vector<cv::Point2f> detectedCircleCenters;


	std::vector<cv::Point> userClickedPoints;
	afx_msg void OnFileUploadimage();
	cv::Mat m_image;
	cv::Mat m_displayImage;
	CString m_imagePath;
	void DisplayImage(CDC* pDC, const cv::Mat& image);
	//void DetectAndDrawCircles(cv::Mat& image);
	bool IsCircle(const std::vector<cv::Point>& points);
	static void OnMouseCallback(int event, int x, int y, int flags, void* userdata);
	void ProcessUserDefinedShape(cv::Mat& image);
	afx_msg void OnCalculateCircleDistances();
	void CalculateDistanceBetweenCenters();
};

#ifndef _DEBUG  // debug version in PCBcircleDetectView.cpp
inline CPCBcircleDetectDoc* CPCBcircleDetectView::GetDocument() const
   { return reinterpret_cast<CPCBcircleDetectDoc*>(m_pDocument); }
#endif

