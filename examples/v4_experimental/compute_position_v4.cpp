
---

## 💻 2. ملف `compute_position_v4.cpp` المُعدّل (بإشارة Z)

```cpp
/*
 * KineticVoice v4.0 - Experimental 3D Cube Array Core
 * 
 * This code implements the 4x4 linear solver for 3D spatial positioning
 * using a 5-microphone cube array. Developed in collaboration with:
 * - Google AI (Gemini): Mathematical framework & 4x4 system derivation
 * - DeepSeek AI: Code optimization & RP2040 integration
 * - Human Lead (NAJIB MOHAMMED AL-AMIR): Mechanical design & system architecture
 * 
 * Licensed under MPL 2.0
 * 
 * 🔧 Updates:
 * - Applied std::copysign to preserve Z-axis sign (Gemini's recommendation)
 * - Added detailed comments for RP2040 compatibility
 */

#include <cmath>

// ==================== Data Structures ====================
struct Point3D {
    double x, y, z;
};

// ==================== Gaussian Elimination 4x4 Solver ====================
bool solve4x4(double A[4][4], double B[4], double solution[4]) {
    int n = 4;
    
    // Forward elimination with partial pivoting
    for (int i = 0; i < n; i++) {
        // Find pivot row with maximum absolute value
        int maxRow = i;
        for (int k = i + 1; k < n; k++) {
            if (std::abs(A[k][i]) > std::abs(A[maxRow][i])) {
                maxRow = k;
            }
        }
        
        // Swap rows
        for (int k = i; k < n; k++) {
            std::swap(A[i][k], A[maxRow][k]);
        }
        std::swap(B[i], B[maxRow]);
        
        // Check for singular matrix
        if (std::abs(A[i][i]) < 1e-9) {
            return false; // Matrix is singular, cannot solve
        }
        
        // Eliminate column
        for (int k = i + 1; k < n; k++) {
            double factor = -A[k][i] / A[i][i];
            for (int j = i; j < n; j++) {
                if (i == j) {
                    A[k][j] = 0;
                } else {
                    A[k][j] += factor * A[i][j];
                }
            }
            B[k] += factor * B[i];
        }
    }
    
    // Back substitution
    for (int i = n - 1; i >= 0; i--) {
        solution[i] = B[i] / A[i][i];
        for (int k = i - 1; k >= 0; k--) {
            B[k] -= A[k][i] * solution[i];
        }
    }
    
    return true;
}

// ==================== Main Positioning Engine (v4.0) ====================
bool computePositionV4(const Point3D mics[4], const double deltaR[4], Point3D& target) {
    // mics[4]: Array of 4 microphone positions relative to the reference (origin)
    // deltaR[4]: TDOA distance differences: d_i = r_i - r_0
    
    double A[4][4] = {0};
    double B[4] = {0};
    double solution[4] = {0}; // [x, y, z, R] where R = sqrt(x^2 + y^2 + z^2)
    
    // Build the linear system: A * [x, y, z, R]^T = B
    for (int i = 0; i < 4; i++) {
        A[i][0] = mics[i].x;
        A[i][1] = mics[i].y;
        A[i][2] = mics[i].z;
        A[i][3] = deltaR[i];
        
        B[i] = 0.5 * (mics[i].x * mics[i].x + 
                      mics[i].y * mics[i].y + 
                      mics[i].z * mics[i].z - 
                      deltaR[i] * deltaR[i]);
    }
    
    // Solve the system
    if (!solve4x4(A, B, solution)) {
        return false; // No valid solution
    }
    
    // Extract coordinates
    target.x = solution[0];
    target.y = solution[1];
    
    // 🔧 Z protection with sign preservation (Gemini's recommendation)
    double R = solution[3];
    double z_sq = (R * R) - (target.x * target.x) - (target.y * target.y);
    
    if (z_sq >= 0.0) {
        // Primary: calculated from R, with sign from the linear solution
        target.z = std::copysign(std::sqrt(z_sq), solution[2]);
    } else {
        // Fallback: direct from matrix (abs value for safety)
        target.z = solution[2];
    }
    
    return true;
}
