#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

const int MAX_VALUE = 255;
// essa var vai ter o valor selecionado pela trackbar
int threshold_value = 0;
// var do tamanho do erode
int dilation_size = 0;
// callback pra quando a trackbar for modificada
void on_change(int, void*);

// frame lido, frame em gray e frame final
Mat frame, frame_gray, frame_blur, frame_thre, frame_final;

int main(int argc, char* argv[]) {
  RNG rand(time(NULL));
  VideoCapture cap(0); // abre a camera n. 0 para captura
  Rect bounding_rect;
  int largest_area = 0, largest_index = -1;
  
  if (!cap.isOpened()) { // testa 
    cout << "Cannot open the video cam" << endl;
    return -1;
  }

  double cWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); // pega a largura dos frames
  double cHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); // pega a altura dos frames

  cout << "Frame size : " << cWidth << " x " << cHeight << endl;

  namedWindow("Threshold", CV_WINDOW_AUTOSIZE); // cria uma janela chamada Threshold
  namedWindow( "Hull output", CV_WINDOW_AUTOSIZE ); // essa jenela vai ter a imagem com o Hull
  createTrackbar("Threshold:", "Threshold", &threshold_value, MAX_VALUE, on_change); // cria a trackbar pra alterar o valor do threshold
  createTrackbar("Dilation:", "Threshold", &dilation_size, MAX_VALUE, on_change); // cria a trackbar pra alterar o valor do threshold

  // laço que irá ler os frames da camera e aplicar o threshold a cada frame e deixa só a pele
  do {
    largest_area = 0;
    largest_index = -1;

    if (!cap.read(frame)) { //if not success, break loop
      cout << "Unable to read frame from video stream!" << endl;
      break;
    }
    
    // converte para tons de cinza
    cvtColor( frame, frame_gray, CV_RGB2GRAY );
    // blur na imagem
    blur( frame_gray, frame_blur, Size(3,3));
    // aplica o threshold
    threshold( frame_blur, frame_thre, threshold_value, 255, CV_THRESH_BINARY_INV);
    // agora aplica erode pra remover ruidos
    // BEGIN
    Mat element = getStructuringElement( MORPH_ELLIPSE,
                                       Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                       Point( dilation_size, dilation_size ) );
          
    // Apply the dilation operation
    dilate(frame_thre, frame_final, element);                             
    // END
    // mostra o frame na janela
    imshow("Threshold", frame_final);
    
    // AGORA AQUI VAI GERAR A IMAGEM DO HULL
    
    // clona a imagem que tomou threshold
    Mat threshold_output = frame_final.clone();
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    /// Find contours
    findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    /// Find the convex hull object for each contour
    vector<vector<Point> >hull( contours.size() );
    for( int i = 0; i < contours.size(); i++ )
    {  convexHull( Mat(contours[i]), hull[i], false ); }
    
    cout << "Size: " << contours.size() << endl;
    
    // pega a maior area
    Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ )
    {
      double a = contourArea(contours[i], false);
      if (a > largest_area) {
        largest_area = a;
        largest_index = i;
        bounding_rect = boundingRect(contours[i]);
      }
    }
    
    // se tiver encontrado um maior então desenha o convex hull e o contorno
    if (largest_index >= 0) {
      Scalar color = Scalar( rand.uniform(0, 255), rand.uniform(0,255), rand.uniform(0,255) );
      drawContours( drawing, contours, largest_index, Scalar(255, 1, 1), 1, 8, vector<Vec4i>(), 0, Point() );
      drawContours( drawing, hull, largest_index, Scalar(255, 255, 255), 1, 8, vector<Vec4i>(), 0, Point() );
      rectangle(drawing, bounding_rect,  Scalar(0,255,0),1, 8,0);
    }
    
    /// Show in a window
    imshow( "Hull output", drawing );

  } while (waitKey(30) != 27);
    
  cout << "End." << endl;
    
  return 0;
}

void on_change(int value, void* data) {
  
}
