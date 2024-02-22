voxel_Float voxel_maths_power(voxel_Float base, voxel_Int power) {
    if (power == 0) {
        return 1;
    }

    if (power < 0) {
        base = 1 / base;
        power *= -1;
    }

    voxel_Float result = base;

    while (power > 1) {
        result *= base;
        power--;
    }

    return result;
}

voxel_Float voxel_maths_roundToPrecision(voxel_Float number, voxel_Count precision) {
    voxel_Bool isNegative = VOXEL_FALSE;

    if (number < 0) {
        isNegative = VOXEL_TRUE;
        number *= -1;
    }

    voxel_UInt integralPart = number;
    voxel_Count integralDigits = 0;

    while (integralPart > 0) {
        integralPart /= 10;
        integralDigits++;
    }

    precision -= integralDigits;

    if (precision < 0) {
        precision = 0;
    }

    voxel_UInt multiplier = voxel_maths_power(10, precision);

    number += 0.5 * voxel_maths_power(10, -precision);

    if (isNegative) {
        number *= -1;
    }

    return (voxel_Float)((voxel_Int)(number * multiplier)) / multiplier;
}