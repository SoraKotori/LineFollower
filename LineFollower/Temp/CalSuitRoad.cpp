#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define theshold_angle 1.57
#define section1 10
#define section2 10
#define right  1
#define middle 0
#define left  -1

void CalSuitRoad(int RoadHeight[2], int RoadArray[2][480], int SuitRoadArray[480], int *SuitEndPoint);
void CorrectRoad(int EndPoint, int Array[480]);
int RoadFirstDeal(int Array[480]);
int RoadFinalDeal(int EndPoint, int section);
double CalThreePoint_Angle(int x1, int y1, int x2, int y2, int x3, int y3);
double CalAverage_Angle(int BeginPoint, int EndPoint, int Array[480], int use[480]);
void DeletErrPoint(int BeginPoint, int EndPoint ,double Average_Angle, int Array[480], int use[480]);
void CalFillPoint(int BeginPoint, int EndPoint, int Array[480]);
void CorrectPoint(int BeginPoint, int EndPoint, double Average_Angle, int Array[480]);
void GetSuitRoad(int RoadHeight[2], int RoadArray[2][480], int SuitRoadArray[480], int *SuitEndPoint);
int CalMidRoadPoint(int BeginPoint, int EndPoint, int Array1[480], int Array2[480]);
int CalAverageRoad_Width(int EndPoint, int Array1[480], int Array2[480]);
void CalFillEndPoint(int second, int EndPoint, int Array[480], int SuitRoadArray[480], int AverageRoad_Width, int *SuitEndPoint);

void CalSuitRoad(int RoadHeight[2], int RoadArray[2][480], int SuitRoadArray[480], int *SuitEndPoint)
{
    CorrectRoad(RoadHeight[0], RoadArray[0]);
    CorrectRoad(RoadHeight[1], RoadArray[1]);
    GetSuitRoad(RoadHeight, RoadArray, SuitRoadArray, *&SuitEndPoint);
}

void CorrectRoad(int EndPoint, int Array[480])
{
    int i;
    int use[480];
    int first  = RoadFirstDeal(Array);
    int second = RoadFinalDeal(EndPoint, section1);

    double angle;

    for(i = first; i < second; i += section1)
    {
        angle = CalAverage_Angle(i, i + section1, Array, use);
        DeletErrPoint(i, i + section1, angle, Array, use);
        angle = CalAverage_Angle(i, i + section1, Array, use);
        CorrectPoint(i, i + section1, angle, Array);
        CalFillPoint(i, i + section1, Array);
    }

    angle = CalAverage_Angle(second, EndPoint, Array, use);
    DeletErrPoint(second, EndPoint, angle, Array, use);
    angle = CalAverage_Angle(second, EndPoint, Array, use);
    CorrectPoint(second, EndPoint, angle, Array);
    CalFillPoint(second, EndPoint, Array);
}

int RoadFirstDeal(int Array[480])
{
    int i;
    double sum;

    for(i = 0; i < section1; i++)
    {
        sum += Array[i];
    }

    Array[0] = (int)round(sum /section1);
    Array[section1-1] = (int)round(sum /section1);

    CalFillPoint(0 , section1-1, Array);

    return section1 - 1;
}

int RoadFinalDeal(int EndPoint, int section)
{
    return EndPoint - (EndPoint % section) - section - 1;
}

double CalThreePoint_Angle(int x1, int y1, int x2, int y2, int x3, int y3)
{
    int x = 0, y = 1;
    int v1[2], v2[2];
    double side1, side2;

    v1[x] = x2 - x1;
    v1[y] = y2 - y1;
    v2[x] = x3 - x1;
    v2[y] = y3 - y1;

    side1 = sqrt(pow((double)v1[x], 2.0) + pow((double)v1[y], 2.0));
    side2 = sqrt(pow((double)v2[x], 2.0) + pow((double)v2[y], 2.0));

    return acos((double)(v1[x] * v2[x] + v1[y] * v2[y]) /  (side1 * side2));
}

double CalAverage_Angle(int BeginPoint, int EndPoint, int Array[480], int use[480])
{
    int i, num = 0;
    double angle_sum = 0;

    for(i = BeginPoint + 1; i < EndPoint; i++)
    {
        if(use[i] == 0)
        {
            num++;
            angle_sum += CalThreePoint_Angle(Array[BeginPoint], BeginPoint, Array[i], i, 1, 0);
        }
    }

    return angle_sum / (double)(num);
}

void DeletErrPoint(int BeginPoint, int EndPoint ,double Average_Angle, int Array[480], int use[480])
{
    int i,angle;

    for(i = BeginPoint + 1; i < EndPoint; i++)
    {
        angle = CalThreePoint_Angle(Array[BeginPoint], BeginPoint, Array[i], i, 1, 0);

        if(theshold_angle < fabs(Average_Angle - angle))
        {
            use[i] = 1;
        }
    }
}

void CalFillPoint(int BeginPoint, int EndPoint, int Array[480])
{
    int i, num;
    double dx;

    num = EndPoint - BeginPoint;
    dx = (Array[EndPoint] - Array[BeginPoint]) / (double)num;

    for(i = BeginPoint + 1; i < EndPoint; i++)
    {
        Array[i] = Array[BeginPoint] + (int)(round(dx * (i - BeginPoint)));
    }
}

void CorrectPoint(int BeginPoint, int EndPoint, double Average_Angle, int Array[480])
{
    double angle;

    angle = CalThreePoint_Angle(Array[BeginPoint], BeginPoint, Array[EndPoint], EndPoint, 1, 0);

    if(theshold_angle < fabs(Average_Angle - angle))
    {
        int dy;

        dy = EndPoint - BeginPoint;

        Array[EndPoint] = (int)round(dy / tan(Average_Angle));
    }
}

void GetSuitRoad(int RoadHeight[2], int RoadArray[2][480], int SuitRoadArray[480], int *SuitEndPoint)
{
    int condition;
    int first  = (section2 * 2) - 1, second;
    int AverageRoad_Width;
    int i;

    if(RoadHeight[1] > RoadHeight[0])
    {
        second = RoadFinalDeal(RoadHeight[1], section2);
        condition = right;
    }
    else if(RoadHeight[0] > RoadHeight[1])
    {
        second = RoadFinalDeal(RoadHeight[0], section2);
        condition = left;
    }
    else
    {
        second = RoadFinalDeal(RoadHeight[0], section2);
        condition = middle;
    }

    SuitRoadArray[0] = (RoadArray[0][0] + RoadArray[1][0]) / 2;
    SuitRoadArray[first] = CalMidRoadPoint(0, first + section2, RoadArray[0], RoadArray[1]);
    CalFillPoint(0, first, SuitRoadArray);

    for(i = first; i < second; i += section2)
    {
        SuitRoadArray[i] = CalMidRoadPoint(i - section2, i + section2, RoadArray[0], RoadArray[1]);
        CalFillPoint(i - section2, i, SuitRoadArray);
    }

    SuitRoadArray[second] = (RoadArray[0][second] + RoadArray[1][second]) / 2;
    CalFillPoint(second - section2, second, SuitRoadArray);



    if(condition == right)
    {
        AverageRoad_Width = CalAverageRoad_Width(RoadHeight[1], RoadArray[0], RoadArray[1]);
        CalFillEndPoint(second, RoadHeight[0], RoadArray[0], SuitRoadArray, AverageRoad_Width, *&SuitEndPoint);
        CalFillPoint(second, *SuitEndPoint, SuitRoadArray);
    }
    else if(condition == left)
    {
        AverageRoad_Width = CalAverageRoad_Width(RoadHeight[0], RoadArray[0], RoadArray[1]);
        CalFillEndPoint(second, RoadHeight[1], RoadArray[1], SuitRoadArray, AverageRoad_Width, *&SuitEndPoint);
        CalFillPoint(second, *SuitEndPoint, SuitRoadArray);
    }
    else
    {
        *SuitEndPoint = 480 - 1;
        SuitRoadArray[RoadHeight[0]] = (RoadArray[0][RoadHeight[0]] + RoadArray[1][RoadHeight[0]]) / 2;
        CalFillPoint(second, RoadHeight[0], SuitRoadArray);
    }


}

int CalMidRoadPoint(int BeginPoint, int EndPoint, int Array1[480], int Array2[480])
{
    int i ,num = 0;
    double sum;

    for(i = BeginPoint; i < EndPoint + 1; i++)
    {
        num += 2;
        sum += Array1[i] + Array2[i];
    }

    return (int)round(sum / num);
}

int CalAverageRoad_Width(int EndPoint, int Array1[480], int Array2[480])
{
    int i, num = 0;
    double sum;

    for(i = 0; i < EndPoint + 1; i++)
    {
        num++;
        sum += abs(Array1[i] - Array2[i]);
    }

    return (int)(round(sum / num) /2);
}

void CalFillEndPoint(int second, int EndPoint, int Array[480], int SuitRoadArray[480], int AverageRoad_Width, int *SuitEndPoint)
{
    int i, point;

    for(i = second; i < EndPoint + 1; i++)
    {
        point = Array[i] + AverageRoad_Width;

        if(point > 640 || point < 0)
        {
            SuitRoadArray[i-1] = Array[i-1] + AverageRoad_Width;
            *SuitEndPoint = i - 1;
        }
    }
}
