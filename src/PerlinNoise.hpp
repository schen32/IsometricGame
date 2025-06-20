#pragma once
#include <vector>

using Noise2DArray = std::vector<std::vector<float>>;
using Noise3DArray = std::vector<std::vector<std::vector<float>>>;

class PerlinNoise
{
public:
    PerlinNoise() = default;

    static Noise2DArray GenerateWhiteNoise(int width, int height)
    {
		srand(time(nullptr));
		Noise2DArray noise(width, std::vector<float>(height));

        for (int i = 0; i < width; i++)
        {
			for (int j = 0; j < height; j++)
			{
				noise[i][j] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			}
        }
        return noise;
    }

    static Noise2DArray GenerateSmoothNoise(const Noise2DArray& baseNoise, int octave)
    {
        int width = baseNoise.size();
        int height = baseNoise[0].size();

		Noise2DArray smoothNoise(width, std::vector<float>(height));

        int samplePeriod = 1 << octave; // calculates 2 ^ k
        float sampleFrequency = 1.0f / samplePeriod;

        for (int i = 0; i < width; i++)
        {
            //calculate the horizontal sampling indices
            int sample_i0 = (i / samplePeriod) * samplePeriod;
            int sample_i1 = (sample_i0 + samplePeriod) % width; //wrap around
            float horizontal_blend = (i - sample_i0) * sampleFrequency;

            for (int j = 0; j < height; j++)
            {
                //calculate the vertical sampling indices
                int sample_j0 = (j / samplePeriod) * samplePeriod;
                int sample_j1 = (sample_j0 + samplePeriod) % height; //wrap around
                float vertical_blend = (j - sample_j0) * sampleFrequency;

                //blend the top two corners
                float top = Interpolate(baseNoise[sample_i0][sample_j0],
                    baseNoise[sample_i1][sample_j0], horizontal_blend);

                //blend the bottom two corners
                float bottom = Interpolate(baseNoise[sample_i0][sample_j1],
                    baseNoise[sample_i1][sample_j1], horizontal_blend);

                //final blend
                smoothNoise[i][j] = Interpolate(top, bottom, vertical_blend);
            }
        }

        return smoothNoise;
    }

    static float Interpolate(float x0, float x1, float alpha)
    {
        return x0 * (1 - alpha) + alpha * x1;
    }

    static Noise2DArray GeneratePerlinNoise(const Noise2DArray& baseNoise, int octaveCount)
    {
        int width = baseNoise.size();
        int height = baseNoise[0].size();

		Noise3DArray smoothNoise(width, std::vector<std::vector<float>>(height));
        float persistance = 0.5f;

        //generate smooth noise
        for (int i = 0; i < octaveCount; i++)
        {
            smoothNoise[i] = GenerateSmoothNoise(baseNoise, i);
        }

		Noise2DArray perlinNoise(width, std::vector<float>(height));
        float amplitude = 1.0f;
        float totalAmplitude = 0.0f;

        //blend noise together
        for (int octave = octaveCount - 1; octave >= 0; octave--)
        {
            amplitude *= persistance;
            totalAmplitude += amplitude;

            for (int i = 0; i < width; i++)
            {
                for (int j = 0; j < height; j++)
                {
                    perlinNoise[i][j] += smoothNoise[octave][i][j] * amplitude;
                }
            }
        }

        //normalisation
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                perlinNoise[i][j] /= totalAmplitude;
            }
        }

        return perlinNoise;
    }
};