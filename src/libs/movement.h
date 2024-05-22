#ifndef MOVEMENT_H_
#define MOVEMENT_H_
typedef enum { left, right } directionLR;

typedef enum { forward, backward } directionFB;

/**
 * @brief Turns robot to specified direction in degrees.
 * @param degrees Amount of degrees to turn.
 * @param d Direction which to turn {left, right}.
 */
void m_turn_degrees(float degrees, directionLR d);
/**
 * @brief Moves given amount in given direction.
 * @param distance Distance to move.
 * @param d Direction which to turn {forward, backward}.
 */
void m_forward_or(float distance, directionFB d);
/**
 * @brief Does waggling moves.
 * @param step_distance Distance covered in single movement.
 * @param d total distance traveled.
 */
void m_waggle(int step_distance, int distance);
/**
 * @brief Initialises motors.
 */
void m_init(void);
/**
 * @brief Extreme stop. Stops on the point
 */
void m_stop(void);
#endif
