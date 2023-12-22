#include "portalManager.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace portal {
    glm::vec2 PortalManager::calculateSurfaceCorrectionVector(glm::vec2 surfaceMin, glm::vec2 surfaceMax, glm::vec2 portalMin, glm::vec2 portalMax) {
        // this function is used to get the correction vector for portals that are outside the surface
        // if the portal is outside the surface, it will snap to within the surface
        // if the portal is inside the surface, just return the 0,0 vector

        glm::vec2 moveVector = {0.0f, 0.0f};
        
        if(portalMin.x < surfaceMin.x){
            // if the portal is outside the surface on the left side
            // move the portal to the right
            moveVector.x = surfaceMin.x - portalMin.x;
        } else if(portalMax.x > surfaceMax.x){
            // if the portal is outside the surface on the right side
            // move the portal to the left
            moveVector.x = surfaceMax.x - portalMax.x;
        }
        if(portalMin.y < surfaceMin.y){
            // if the portal is outside the surface on the bottom side
            // move the portal up
            moveVector.y = surfaceMin.y - portalMin.y;
        } else if(portalMax.y > surfaceMax.y){
            // if the portal is outside the surface on the top side
            // move the portal down
            moveVector.y = surfaceMax.y - portalMax.y;
        }

        return moveVector;
    }
    
    glm::vec2 PortalManager::lineLineIntersection(glm::vec2 A, glm::vec2 B, glm::vec2 C, glm::vec2 D) 
    { 
        // Line AB represented as a1x + b1y = c1 
        double a1 = B.y - A.y; 
        double b1 = A.x - B.x; 
        double c1 = a1*(A.x) + b1*(A.y); 
    
        // Line CD represented as a2x + b2y = c2 
        double a2 = D.y - C.y; 
        double b2 = C.x - D.x; 
        double c2 = a2*(C.x)+ b2*(C.y); 
    
        double determinant = a1*b2 - a2*b1; 
    
        if (determinant == 0) 
        { 
            // The lines are parallel. This is simplified
            // by returning a pair of FLT_MAX
            return {FLT_MAX, FLT_MAX};
        } 
        else
        { 
            double x = (b2*c1 - b1*c2)/determinant; 
            double y = (a1*c2 - a2*c1)/determinant; 
            return {x, y};
        } 
    }

    bool PortalManager::isPointInsideRectangle(const glm::vec2& point, const Rectangle& rectangle){
        glm::vec2 vec1 = point - rectangle.topLeft;
        glm::vec2 vec2 = point - rectangle.topRight;
        glm::vec2 vec3 = point - rectangle.bottomRight;
        glm::vec2 vec4 = point - rectangle.bottomLeft;

        glm::vec2 side1 = rectangle.topRight - rectangle.topLeft;
        glm::vec2 side2 = rectangle.bottomRight - rectangle.topRight;
        glm::vec2 side3 = rectangle.bottomLeft - rectangle.bottomRight;
        glm::vec2 side4 = rectangle.topLeft - rectangle.bottomLeft;

        float cross1 = glm::cross(glm::vec3(vec1, 0.0), glm::vec3(side1, 0.0)).z;
        float cross2 = glm::cross(glm::vec3(vec2, 0.0), glm::vec3(side2, 0.0)).z;
        float cross3 = glm::cross(glm::vec3(vec3, 0.0), glm::vec3(side3, 0.0)).z;
        float cross4 = glm::cross(glm::vec3(vec4, 0.0), glm::vec3(side4, 0.0)).z;

        // Check if all cross products have the same sign
        return (cross1 >= 0 && cross2 >= 0 && cross3 >= 0 && cross4 >= 0) || (cross1 <= 0 && cross2 <= 0 && cross3 <= 0 && cross4 <= 0);
    }

    std::vector<glm::vec2> PortalManager::getLineRectangleIntersectionPoints(Rectangle& portal, Rectangle& otherPortal, Point point){
        // this function is used to get the intersection points of the line between the centers of the 2 portals
        // with the sides of the bounding box of the portals depending on the point passed to the function (the point inside the other portal)
        // portal is the one that is being shot/casted onto a surface
        // otherPortal is the one that is already shot/casted onto a surface
        glm::vec2 intersectionPoints[4];
        std::vector<std::pair<glm::vec2, glm::vec2>> sides(4);
        switch (point)
        {
        case Point::TOP_LEFT:
            sides[0] = {otherPortal.bottomRight, otherPortal.topRight};
            sides[1] = {otherPortal.bottomLeft, otherPortal.bottomRight};
            sides[2] = {portal.bottomLeft, portal.topLeft};
            sides[3] = {portal.topLeft, portal.topRight};
            break;
        case Point::TOP_RIGHT:
            sides[0] = {otherPortal.bottomLeft, otherPortal.topLeft};
            sides[1] = {otherPortal.bottomLeft, otherPortal.bottomRight};
            sides[2] = {portal.bottomRight, portal.topRight};
            sides[3] = {portal.topLeft, portal.topRight};
            break;
        case Point::BOTTOM_LEFT:
            sides[0] = {otherPortal.bottomRight, otherPortal.topRight};
            sides[1] = {otherPortal.topLeft, otherPortal.topRight};
            sides[2] = {portal.bottomLeft, portal.topLeft};
            sides[3] = {portal.bottomLeft, portal.bottomRight};
            break;
        case Point::BOTTOM_RIGHT:
            sides[0] = {otherPortal.bottomLeft, otherPortal.topLeft};
            sides[1] = {otherPortal.topLeft, otherPortal.topRight};
            sides[2] = {portal.bottomRight, portal.topRight};
            sides[3] = {portal.bottomLeft, portal.bottomRight};
            break;
        default:
            break;
        }

        for(int i = 0; i < 4; i++){
            intersectionPoints[i] = lineLineIntersection(otherPortal.center, portal.center, sides[i].first, sides[i].second);
        }

        float distance1 = glm::distance(intersectionPoints[0], otherPortal.center);
        float distance2 = glm::distance(intersectionPoints[1], otherPortal.center);
        float distance3 = glm::distance(intersectionPoints[2], portal.center);
        float distance4 = glm::distance(intersectionPoints[3], portal.center);
        
        intersectionPoints[0] = distance1 < distance2 ? intersectionPoints[0] : intersectionPoints[1];
        intersectionPoints[1] = distance3 < distance4 ? intersectionPoints[2] : intersectionPoints[3];

        // return intersection points 0 and 1 only since they are the closest to the portals
        std::vector<glm::vec2> result(2);
        result[0] = intersectionPoints[0];
        result[1] = intersectionPoints[1];
        return result;


    }

    glm::vec2 PortalManager::calculatePortalOverlapCorrectionVector(Rectangle& portal, Rectangle& otherPortal){
        // this function is used to get the correction vector for portals that are overlapping
        // if the portal is overlapping the other portal, it will move the portal out of the other portal
        // if the portal is not overlapping the other portal, just return the 0,0 vector
        
        // portal is the one that is being shot/casted onto a surface
        // otherPortal is the one that is already shot/casted onto a surface
        
        // To move the portal out of the other portal, 
        // we get the point of intersection of the rectangle sides with the line between the centers of the 2 portals
        // then we move along that line with a distance of the distance between the intersection points
        // so that at the end both intersection points are on the same line
        /*
            example:
            |--------------|-----|-----------------| 
            |              |     |                 |
            |              |     |                 |
            |              |     |                 |
            |        c     |<=d=>|        c        |
            |              |     |                 |
            |              |     |                 |
            |              |     |                 |
            |--------------|-----|-----------------|
            Here is the portal on the left and the other portal on the right. They are overlapping. 
            If we join the 2 centers of the portals and get the intersection points of the sides of the portals with that line
            we would find the distance between the intersection points and move the portal along the line by that distance
            and the portals would not be overlapping anymore.
            There might be some issues if the rectangles are rotated, but that would only cause the edges of the portals
            to overlap, which is not a problem since our portals are rendered as ovals anyway.
        */
        glm::vec2 moveVector = {0.0f, 0.0f};
        // for the top left of the portal to be inside the other portal, the portal must be on the right side of the other portal
        if(isPointInsideRectangle(portal.topLeft, otherPortal)){ 
            // since the top left of the portal is inside the other portal, 
            // we know that the vector intersects either the right or bottom side of the other portal
            // and the vector intersects either the left or top side of the portal

            // get intersection points of the vector with the suspected sides of the portals
            std::vector<glm::vec2> intersectionPoints = getLineRectangleIntersectionPoints(portal, otherPortal, Point::TOP_LEFT);

            moveVector = intersectionPoints[0] - intersectionPoints[1];
            // if move vector is in the same direction as vector from other portal center to portal center
            // then return move vector. 
            // this condition is to fix a bug where at a specific angle, both top left and top right are inside the other portal
            // but since the top left is checked first, the move vector is calculated from the top left and the portal is moved in the wrong direction
            if(glm::dot(moveVector, portal.center - otherPortal.center) > 0.1){
                return moveVector;
            }
        } 
        if(isPointInsideRectangle(portal.topRight, otherPortal)){
            // since the top right of the portal is inside the other portal,
            // we know that the vector intersects either the left or bottom side of the other portal
            // and the vector intersects either the right or top side of the portal

            // get intersection points of the vector with the suspected sides of the portals
            std::vector<glm::vec2> intersectionPoints = getLineRectangleIntersectionPoints(portal, otherPortal, Point::TOP_RIGHT);

            moveVector = intersectionPoints[0] - intersectionPoints[1];
            return moveVector;

        } 
        if(isPointInsideRectangle(portal.bottomLeft, otherPortal)){
            // since the bottom left of the portal is inside the other portal,
            // we know that the vector intersects either the right or top side of the other portal
            // and the vector intersects either the left or bottom side of the portal

            // get intersection points of the vector with the suspected sides of the portals
            std::vector<glm::vec2> intersectionPoints = getLineRectangleIntersectionPoints(portal, otherPortal, Point::BOTTOM_LEFT);

            moveVector = intersectionPoints[0] - intersectionPoints[1];
            if(glm::dot(moveVector, portal.center - otherPortal.center) > 0.1){
                return moveVector;
            }
        }  
        if(isPointInsideRectangle(portal.bottomRight, otherPortal)){
            // since the bottom right of the portal is inside the other portal,
            // we know that the vector intersects either the left or top side of the other portal
            // and the vector intersects either the right or bottom side of the portal

            // get intersection points of the vector with the suspected sides of the portals
            std::vector<glm::vec2> intersectionPoints = getLineRectangleIntersectionPoints(portal, otherPortal, Point::BOTTOM_RIGHT);

            moveVector = intersectionPoints[0] - intersectionPoints[1];
        }
        return moveVector;

    }

    bool PortalManager::getCorrectedPortalPos(Entity* surface, Portal* otherPortal, glm::vec3& hitPoint, glm::vec3 up, glm::vec3 right, glm::vec3 front) {
        // this function is used to get the corrected position of the portal
        // portal is the one that is being shot/casted onto a surface
        // otherPortal is the one that is already shot/casted onto a surface
        // if the portal is outside the surface, it will snap to within the surface
        // if the portal is overlapping the other portal, it will move the portal out of the other portal
        // if the portal can't be placed inside the surface and not overlap the other portal, it will return true
        // if the portal can be placed inside the surface and not overlap the other portal, it will return false        

        // vertices of the rectangles
        glm::vec3 otherPortalPoints[4];
        glm::vec3 portalPoints[4];
        if(otherPortal){
            // get the bounding box of the other portal
            glm::vec3 portal1Up = glm::vec3(otherPortal->portalRot * glm::vec3(0,1,0));
            glm::vec3 portal1Right = glm::vec3(otherPortal->portalRot * glm::vec3(1,0,0));

            otherPortalPoints[0] = otherPortal->portalPosition + portal1Up * 2.62f - portal1Right * 1.78f;
            otherPortalPoints[1] = otherPortal->portalPosition + portal1Up * 2.62f + portal1Right * 1.78f;
            otherPortalPoints[2] = otherPortal->portalPosition - portal1Up * 2.62f - portal1Right * 1.78f;
            otherPortalPoints[3] = otherPortal->portalPosition - portal1Up * 2.62f + portal1Right * 1.78f;

            // get the bounding box of the portal
            portalPoints[0] = hitPoint + up * 2.62f - right * 1.78f;
            portalPoints[1] = hitPoint + up * 2.62f + right * 1.78f;
            portalPoints[2] = hitPoint - up * 2.62f - right * 1.78f;
            portalPoints[3] = hitPoint - up * 2.62f + right * 1.78f;
        }

        
        // Get the minimum and maximum points of the surface and the portal
        // this is used to check if the portal is inside the surface

        // get the bounding box of the surface
        r3d::Vector3 surfaceMinR3d, surfaceMaxR3d;
        surface->getComponent<RigidBodyComponent>()->getCollider()->getCollisionShape()->getLocalBounds(surfaceMinR3d, surfaceMaxR3d);
        glm::vec4 surfaceMin(surfaceMinR3d.x, surfaceMinR3d.y, surfaceMinR3d.z, 1.0);
        glm::vec4 surfaceMax(surfaceMaxR3d.x, surfaceMaxR3d.y, surfaceMaxR3d.z, 1.0);
        
        float matrix[16];
        surface->localTransform.getTransform().getOpenGLMatrix(matrix);
        glm::mat4 noscale = glm::make_mat4(matrix);

        surfaceMin = noscale * surfaceMin;
        surfaceMax = noscale * surfaceMax;
        glm::vec4 temp = surfaceMax;
        surfaceMax = glm::vec4(std::max(surfaceMax.x, surfaceMin.x), std::max(surfaceMax.y, surfaceMin.y), std::max(surfaceMax.z, surfaceMin.z), 1.0);
        surfaceMin = glm::vec4(std::min(temp.x, surfaceMin.x), std::min(temp.y, surfaceMin.y), std::min(temp.z, surfaceMin.z), 1.0);

        glm::vec3 portalMin = glm::vec3(hitPoint.x - std::abs(right.x) * 1.78f - std::abs(up.x) * 2.62f, 
                                        hitPoint.y - std::abs(right.y) * 1.78f - std::abs(up.y) * 2.62f, 
                                        hitPoint.z - std::abs(right.z) * 1.78f - std::abs(up.z) * 2.62f);

        glm::vec3 portalMax = glm::vec3(hitPoint.x + std::abs(right.x) * 1.78f + std::abs(up.x) * 2.62f, 
                                        hitPoint.y + std::abs(right.y) * 1.78f + std::abs(up.y) * 2.62f, 
                                        hitPoint.z + std::abs(right.z) * 1.78f + std::abs(up.z) * 2.62f);
        if(std::abs(glm::dot(front, glm::vec3(0,0,1))) > 0.9){
            // if the portal is on the z plane
            // then we can work with the x and y coordinates only

            // move the portal to the surface if it is outside the surface
            glm::vec2 moveVector = calculateSurfaceCorrectionVector(glm::vec2(surfaceMin.x, surfaceMin.y), glm::vec2(surfaceMax.x, surfaceMax.y), 
                                                                    glm::vec2(portalMin.x, portalMin.y), glm::vec2(portalMax.x, portalMax.y));
            
            // update the hitPoint and the portalMin and portalMax, and the portalPoints array
            hitPoint += glm::vec3(moveVector.x, moveVector.y, 0.0f );

            // if the other portal is not shot yet or the portals are not on the same plane then just return.
            if(!otherPortal || (glm::dot(otherPortal->portalNormal, glm::vec4(front,0.0))) < 0.9) return false;

            portalMin += glm::vec3(moveVector.x, moveVector.y, 0.0f );
            portalMax += glm::vec3(moveVector.x, moveVector.y, 0.0f );
            for(int i = 0; i < 4; i++){
                portalPoints[i] += glm::vec3(moveVector.x, moveVector.y, 0.0f );
            }


            // sort the points of the portals according to the y value

            std::sort(otherPortalPoints, otherPortalPoints + 4, [](glm::vec3 a, glm::vec3 b) {
                return a.y > b.y;
            });

            std::sort(portalPoints, portalPoints + 4, [](glm::vec3 a, glm::vec3 b) {
                return a.y > b.y;
            });

            // now the first 2 points are the top right and top left of the portals

            Rectangle portal1;
            portal1.topLeft = (otherPortalPoints[0].x < otherPortalPoints[1].x) ? otherPortalPoints[0] : otherPortalPoints[1];
            portal1.topRight = (otherPortalPoints[0].x > otherPortalPoints[1].x) ? otherPortalPoints[0] : otherPortalPoints[1];
            portal1.bottomLeft = (otherPortalPoints[2].x < otherPortalPoints[3].x) ? otherPortalPoints[2] : otherPortalPoints[3];
            portal1.bottomRight = (otherPortalPoints[2].x > otherPortalPoints[3].x) ? otherPortalPoints[2] : otherPortalPoints[3];
            portal1.center = {otherPortal->portalPosition.x, otherPortal->portalPosition.y};

            // check if hitPoint is inside the other portal
            if(isPointInsideRectangle(glm::vec2(hitPoint.x, hitPoint.y), portal1)){
                // if it is inside, return true
                return true;
            }

            Rectangle portal2;
            portal2.topLeft = (portalPoints[0].x < portalPoints[1].x) ? portalPoints[0] : portalPoints[1];
            portal2.topRight = (portalPoints[0].x > portalPoints[1].x) ? portalPoints[0] : portalPoints[1];
            portal2.bottomLeft = (portalPoints[2].x < portalPoints[3].x) ? portalPoints[2] : portalPoints[3];
            portal2.bottomRight = (portalPoints[2].x > portalPoints[3].x) ? portalPoints[2] : portalPoints[3];
            portal2.center = {hitPoint.x, hitPoint.y};


            // check if the portals are overlapping
            moveVector = calculatePortalOverlapCorrectionVector(portal2, portal1);
            if(moveVector != glm::vec2(0.0f, 0.0f)){
                hitPoint += glm::vec3(moveVector.x, moveVector.y, 0.0f );
                portalMin += glm::vec3(moveVector.x, moveVector.y, 0.0f );
                portalMax += glm::vec3(moveVector.x, moveVector.y, 0.0f );
                // return true;
            } else{
                // if there is no points from the portal inside the other portal
                // check if there is any points from the other portal inside the portal
                moveVector = calculatePortalOverlapCorrectionVector(portal1, portal2);
                hitPoint -= glm::vec3(moveVector.x, moveVector.y, 0.0f );
                portalMin -= glm::vec3(moveVector.x, moveVector.y, 0.0f );
                portalMax -= glm::vec3(moveVector.x, moveVector.y, 0.0f );
            }

            // check if the portal is outside the surface
            // if it is outside, then it can't be placed inside the surface and not overlap the other portal
            // so just return true
            moveVector = calculateSurfaceCorrectionVector(glm::vec2(surfaceMin.x, surfaceMin.y), glm::vec2(surfaceMax.x, surfaceMax.y), 
                                                        glm::vec2(portalMin.x, portalMin.y), glm::vec2(portalMax.x, portalMax.y));
            if(moveVector != glm::vec2(0.0f, 0.0f)){
                return true;
            }

        } 
        else if(std::abs(glm::dot(front, glm::vec3(1,0,0))) > 0.9){
            // if the portal is on the x plane
            // then we can work with the y and z coordinates only
            // move the portal to the surface if it is outside the surface
            glm::vec2 moveVector = calculateSurfaceCorrectionVector(glm::vec2(surfaceMin.z, surfaceMin.y), glm::vec2(surfaceMax.z, surfaceMax.y), 
                                                                    glm::vec2(portalMin.z, portalMin.y), glm::vec2(portalMax.z, portalMax.y));
            
            // update the hitPoint and the portalMin and portalMax, and the portalPoints array
            hitPoint += glm::vec3(0.0f, moveVector.y, moveVector.x);

            // if the other portal is not shot yet or the portals are not on the same plane then just return.
            if(!otherPortal || (glm::dot(otherPortal->portalNormal, glm::vec4(front,0.0))) < 0.9) return false;
            
            portalMin += glm::vec3(0.0f, moveVector.y, moveVector.x);
            portalMax += glm::vec3(0.0f, moveVector.y, moveVector.x);
            for(int i = 0; i < 4; i++){
                portalPoints[i] += glm::vec3(0.0f, moveVector.y, moveVector.x);
            }


            // sort the points of the portals according to the y value

            std::sort(otherPortalPoints, otherPortalPoints + 4, [](glm::vec3 a, glm::vec3 b) {
                return a.y > b.y;
            });

            std::sort(portalPoints, portalPoints + 4, [](glm::vec3 a, glm::vec3 b) {
                return a.y > b.y;
            });

            // now the first 2 points are the top right and top left of the portals

            Rectangle portal1;
            portal1.topLeft = (otherPortalPoints[0].z < otherPortalPoints[1].z) ? glm::vec2(otherPortalPoints[0].z, otherPortalPoints[0].y) : glm::vec2(otherPortalPoints[1].z, otherPortalPoints[1].y);
            portal1.topRight = (otherPortalPoints[0].z > otherPortalPoints[1].z) ? glm::vec2(otherPortalPoints[0].z, otherPortalPoints[0].y) : glm::vec2(otherPortalPoints[1].z, otherPortalPoints[1].y);
            portal1.bottomLeft = (otherPortalPoints[2].z < otherPortalPoints[3].z) ? glm::vec2(otherPortalPoints[2].z, otherPortalPoints[2].y) : glm::vec2(otherPortalPoints[3].z, otherPortalPoints[3].y);
            portal1.bottomRight = (otherPortalPoints[2].z > otherPortalPoints[3].z) ? glm::vec2(otherPortalPoints[2].z, otherPortalPoints[2].y) : glm::vec2(otherPortalPoints[3].z, otherPortalPoints[3].y);
            portal1.center = {otherPortal->portalPosition.z, otherPortal->portalPosition.y};

            // check if hitPoint is inside the other portal
            if(isPointInsideRectangle(glm::vec2(hitPoint.z, hitPoint.y), portal1)){
                // if it is inside, return true
                return true;
            }

            Rectangle portal2;
            portal2.topLeft = (portalPoints[0].z < portalPoints[1].z) ? glm::vec2(portalPoints[0].z, portalPoints[0].y) : glm::vec2(portalPoints[1].z, portalPoints[1].y);
            portal2.topRight = (portalPoints[0].z > portalPoints[1].z) ? glm::vec2(portalPoints[0].z, portalPoints[0].y) : glm::vec2(portalPoints[1].z, portalPoints[1].y);
            portal2.bottomLeft = (portalPoints[2].z < portalPoints[3].z) ? glm::vec2(portalPoints[2].z, portalPoints[2].y) : glm::vec2(portalPoints[3].z, portalPoints[3].y);
            portal2.bottomRight = (portalPoints[2].z > portalPoints[3].z) ? glm::vec2(portalPoints[2].z, portalPoints[2].y) : glm::vec2(portalPoints[3].z, portalPoints[3].y);
            portal2.center = {hitPoint.z, hitPoint.y};
            
            // check if the portals are overlapping
            moveVector = calculatePortalOverlapCorrectionVector(portal2, portal1);
            if(moveVector != glm::vec2(0.0f, 0.0f)){
                hitPoint += glm::vec3(0.0f, moveVector.y, moveVector.x);
                portalMin += glm::vec3(0.0f, moveVector.y, moveVector.x);
                portalMax += glm::vec3(0.0f, moveVector.y, moveVector.x);
                
                // return true;
            } else{
                // if there is no points from the portal inside the other portal
                // check if there is any points from the other portal inside the portal
                moveVector = calculatePortalOverlapCorrectionVector(portal1, portal2);
                hitPoint -= glm::vec3(0.0f, moveVector.y, moveVector.x);
                portalMin -= glm::vec3(0.0f, moveVector.y, moveVector.x);
                portalMax -= glm::vec3(0.0f, moveVector.y, moveVector.x);
            }

            // check if the portal is outside the surface
            // if it is outside, then it can't be placed inside the surface and not overlap the other portal
            // so just return true
            moveVector = calculateSurfaceCorrectionVector(glm::vec2(surfaceMin.z, surfaceMin.y), glm::vec2(surfaceMax.z, surfaceMax.y), 
                                                        glm::vec2(portalMin.z, portalMin.y), glm::vec2(portalMax.z, portalMax.y));
            if(moveVector != glm::vec2(0.0f, 0.0f)){
                return true;
            }
        } else if(std::abs(glm::dot(front, glm::vec3(0,1,0))) > 0.9){
            // if the portal is on the y plane
            // then we can work with the x and z coordinates only

            // move the portal to the surface if it is outside the surface
            glm::vec2 moveVector = calculateSurfaceCorrectionVector(glm::vec2(surfaceMin.x, surfaceMin.z), glm::vec2(surfaceMax.x, surfaceMax.z), 
                                                        glm::vec2(portalMin.x, portalMin.z), glm::vec2(portalMax.x, portalMax.z));

            // update the hitPoint and the portalMin and portalMax, and the portalPoints array
            hitPoint += glm::vec3(moveVector.x, 0.0f, moveVector.y);

            // if the other portal is not shot yet or the portals are not on the same plane then just return.
            if(!otherPortal || (glm::dot(otherPortal->portalNormal, glm::vec4(front,0.0))) < 0.9) return false;

            portalMin += glm::vec3(moveVector.x, 0.0f, moveVector.y);
            portalMax += glm::vec3(moveVector.x, 0.0f, moveVector.y);
            for(int i = 0; i < 4; i++){
                portalPoints[i] += glm::vec3(moveVector.x, 0.0f, moveVector.y);
            }

            // sort the points of the portals according to the z value

            std::sort(otherPortalPoints, otherPortalPoints + 4, [](glm::vec3 a, glm::vec3 b) {
                return a.z > b.z;
            });

            std::sort(portalPoints, portalPoints + 4, [](glm::vec3 a, glm::vec3 b) {
                return a.z > b.z;
            });

            // now the first 2 points are the top right and top left of the portals

            Rectangle portal1;
            portal1.topLeft = (otherPortalPoints[0].x < otherPortalPoints[1].x) ? glm::vec2(otherPortalPoints[0].x, otherPortalPoints[0].z) : glm::vec2(otherPortalPoints[1].x, otherPortalPoints[1].z);
            portal1.topRight = (otherPortalPoints[0].x > otherPortalPoints[1].x) ? glm::vec2(otherPortalPoints[0].x, otherPortalPoints[0].z) : glm::vec2(otherPortalPoints[1].x, otherPortalPoints[1].z);
            portal1.bottomLeft = (otherPortalPoints[2].x < otherPortalPoints[3].x) ? glm::vec2(otherPortalPoints[2].x, otherPortalPoints[2].z) : glm::vec2(otherPortalPoints[3].x, otherPortalPoints[3].z);
            portal1.bottomRight = (otherPortalPoints[2].x > otherPortalPoints[3].x) ? glm::vec2(otherPortalPoints[2].x, otherPortalPoints[2].z) : glm::vec2(otherPortalPoints[3].x, otherPortalPoints[3].z);
            portal1.center = glm::vec2(otherPortal->portalPosition.x, otherPortal->portalPosition.z);

            // check if hitPoint is inside the other portal
            if(isPointInsideRectangle(glm::vec2(hitPoint.x, hitPoint.z), portal1)){
                // if it is inside, return true
                return true;
            }
            
            Rectangle portal2;
            portal2.topLeft = (portalPoints[0].x < portalPoints[1].x) ? glm::vec2(portalPoints[0].x, portalPoints[0].z) : glm::vec2(portalPoints[1].x, portalPoints[1].z);
            portal2.topRight = (portalPoints[0].x > portalPoints[1].x) ? glm::vec2(portalPoints[0].x, portalPoints[0].z) : glm::vec2(portalPoints[1].x, portalPoints[1].z);
            portal2.bottomLeft = (portalPoints[2].x < portalPoints[3].x) ? glm::vec2(portalPoints[2].x, portalPoints[2].z) : glm::vec2(portalPoints[3].x, portalPoints[3].z);
            portal2.bottomRight = (portalPoints[2].x > portalPoints[3].x) ? glm::vec2(portalPoints[2].x, portalPoints[2].z) : glm::vec2(portalPoints[3].x, portalPoints[3].z);
            portal2.center = glm::vec2(hitPoint.x, hitPoint.z);


            // check if the portals are overlapping
            moveVector = calculatePortalOverlapCorrectionVector(portal2, portal1);
            if(moveVector != glm::vec2(0.0f, 0.0f)){
                hitPoint += glm::vec3(moveVector.x, 0.0f, moveVector.y);
                portalMin += glm::vec3(moveVector.x, 0.0f, moveVector.y);
                portalMax += glm::vec3(moveVector.x, 0.0f, moveVector.y);
                // return true;
            } else{
                // if there is no points from the portal inside the other portal
                // check if there is any points from the other portal inside the portal
                moveVector = calculatePortalOverlapCorrectionVector(portal1, portal2);
                hitPoint -= glm::vec3(moveVector.x, 0.0f, moveVector.y);
                portalMin -= glm::vec3(moveVector.x, 0.0f, moveVector.y);
                portalMax -= glm::vec3(moveVector.x, 0.0f, moveVector.y);
            }
            
            // check if the portal is outside the surface
            // if it is outside, then it can't be placed inside the surface and not overlap the other portal
            // so just return true
            moveVector = calculateSurfaceCorrectionVector(glm::vec2(surfaceMin.x, surfaceMin.z), glm::vec2(surfaceMax.x, surfaceMax.z), 
                                                        glm::vec2(portalMin.x, portalMin.z), glm::vec2(portalMax.x, portalMax.z));
            if(moveVector != glm::vec2(0.0f, 0.0f)){
                return true;
            }
        }
        return false;
    }

    bool PortalManager::castPortal(Entity* surface, Portal* portal, glm::vec3 hitPoint) {
        // place portal on the surface with correct orientation
        
        // step 1 set the position of the portal to the position of the surface
        // get the surface location and normal
        r3d::Vector3 surfaceLocation = surface->localTransform.getPosition();
        r3d::Vector3 surfaceNormal = surface->localTransform.getRotation() * r3d::Vector3(0,0,1);
        glm::vec3 surfaceLocationGlm = {surfaceLocation.x, surfaceLocation.y, surfaceLocation.z};
        glm::vec3 surfaceNormalGlm = {surfaceNormal.x, surfaceNormal.y, surfaceNormal.z};

        // step 2 set the orientation of the portal correctly
        // to set the orientation we need to know the up vector of the portal and its normal
        // the normal is the same as the surface normal
        // to set the up vector we need to check if the surface is a floor, ceiling, wall
        // if it is a floor or ceiling we need to project the player position on the surface 
        // then calculate the vector from the raycast hit to the player projection on the surface
        // that vector is the down vector of the portal and its negative is the up vector
        // and the front vector is the normal of the surface
        // if it is a wall then the up vector is the global up vector and the front vector is the normal of the surface
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        if(std::abs(glm::dot(surfaceNormalGlm, glm::vec3(0,1,0))) > 0.9) {
            // if the surface is a floor or ceiling
            // get the player position
            glm::vec3 playerPosition = {player->localTransform.getPosition().x, player->localTransform.getPosition().y, player->localTransform.getPosition().z};
            // get the vector from the raycast hit to the player
            glm::vec3 surfaceToPlayer = playerPosition - hitPoint;
            // project the player position on the surface
            glm::vec3 projectedPlayerPosition = playerPosition - glm::dot(surfaceToPlayer, surfaceNormalGlm) * surfaceNormalGlm;
            // get the down vector of the portal
            glm::vec3 down = projectedPlayerPosition - hitPoint;
            // get the up vector of the portal
            up =  glm::normalize(-down);
                                
        } 
        glm::vec3 front = surfaceNormalGlm;
        glm::vec3 right = glm::normalize(glm::cross(up, front));
        
                
        Portal* otherPortal =  nullptr;
        if(portal == Portal_1){
            if(isPortal2Shot) otherPortal = Portal_2;
        } else {
            if(isPortal1Shot) otherPortal = Portal_1;
        }

        if(getCorrectedPortalPos(surface, otherPortal, hitPoint, up, right, front)){
            return false;
        }

        glm::mat4 orientation = glm::mat4(glm::vec4(right, 0.0f), glm::vec4(up, 0.0f), glm::vec4(front, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        
        glm::quat orientationQuat = glm::quat_cast(orientation);
        orientationQuat = glm::normalize(orientationQuat);
        r3d::Quaternion orientationR3d = r3d::Quaternion(orientationQuat.x, orientationQuat.y, orientationQuat.z, orientationQuat.w);
        
        r3d::Vector3 pos = r3d::Vector3(hitPoint.x, hitPoint.y, hitPoint.z);
        pos += surfaceNormal * 0.1f;
        // set the orientation of the portal
        portal->localTransform.setTransform(r3d::Transform(pos, orientationR3d));
        portal->getComponent<RigidBodyComponent>()->getBody()->setTransform(r3d::Transform(pos + Portal_1->getComponent<RigidBodyComponent>()->relativePosition, orientationR3d));
        
        // calculate the cached values of the portal (e.g. the localToWorld matrix)
        portal->calculateCachedValues();
        // gets the surface behind the portal and sets the surface data memeber
        portal->getSurface();

        return true;
    }

    void PortalManager::checkPortalShot(){
         // Check if Mouse left is pressed
        if(app->getMouse().justPressed(GLFW_MOUSE_BUTTON_1)) {
            std::string name = "";
            glm::vec3 hitPoint = glm::vec3(0.0f, 0.0f, 0.0f);
            // RayCast from player position to front direction with length 50
            r3d::Ray ray(player->localTransform.getPosition() + player->getAbsoluteFront() / 2.0f,player->getAbsoluteFront() * 50 + player->localTransform.getPosition());
            physicsWorld->raycast(ray, new RayCastPortal(name, hitPoint));
            if(name.empty()) return;
            // get entity with current name and check if it is can hold a portal
            Entity *potentialPortalSurface = player->getWorld()->getEntityByName(name);
            if(potentialPortalSurface->canHoldPortal) {
                // if portal can be placed, place it
                if(!castPortal(potentialPortalSurface, Portal_1, hitPoint)) return;
                
                isPortal1Shot = true;

                if(isPortal2Shot){
                    Portal_1->setDestination(Portal_2);
                    Portal_2->setDestination(Portal_1);
                }
            }

        } else if (app->getMouse().justPressed(GLFW_MOUSE_BUTTON_2)) {
            std::string name = "";
            glm::vec3 hitPoint = glm::vec3(0.0f, 0.0f, 0.0f);
            // RayCast from player position to front direction with length 50
            r3d::Ray ray(player->localTransform.getPosition() + player->getAbsoluteFront(), player->getAbsoluteFront() * 50 + player->localTransform.getPosition());
            physicsWorld->raycast(ray, new RayCastPortal(name, hitPoint));
            if(name.empty()) return;
            // get entity with current name and check if it can hold a portal
            Entity *potentialPortalSurface = player->getWorld()->getEntityByName(name);
            
            // if portal can be placed, place it
            if(potentialPortalSurface->canHoldPortal) {
                if(!castPortal(potentialPortalSurface, Portal_2, hitPoint)) return;

                isPortal2Shot = true;

                // if both portals are shot, set the destination of each portal to the other
                if(isPortal1Shot){
                    Portal_1->setDestination(Portal_2);
                    Portal_2->setDestination(Portal_1);
                }
            }
        }
    }

}