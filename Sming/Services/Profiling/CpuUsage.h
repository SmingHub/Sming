#include <Platform/System.h>
#include <Platform/Timers.h>

namespace Profiling
{
/**
 * @brief Class to provide a CPU usage indication based on task callback availability.
 * @note To use, instantiate a single instance of this class and call the `begin()` method
 * from `init()`, passing a function to be invoked after calibration has completed.
 *
 * @note CPU usage is calculated over an update period which begins with a call to reset().
 * The actual update period must be managed elsewhere, using a callback timer, web request
 * or other mechanism. It doesn't need to be exact as the actual elapsed time in CPU
 * cycles is used for the calculation.
 * After the update period has elapsed, call `getUtilisation()` to obtain a CPU usage figure.
 *
 * This figure is obtained using the number of task callbacks made within the update period.
 *
 * 	loop cycles:	Set up repeating task callback and invocations between successive calls
 *	total cycles:	The total number of CPU cycles between calls to update()
 *	used:			total - loop
 *  utilisation:	used / total
 */
class CpuUsage
{
public:
	/**
	 * @brief Calibrate the baseline figure for minimum CPU usage
	 * @param ready Function to call when calibration is complete
	 * @note Typically call this in `init()`
	 */
	void begin(InterruptCallback ready)
	{
		onReady = ready;
		queueCalibrationLoop();
	}

	/**
	 * @brief Reset counters to start a new update period
	 */
	void reset()
	{
		cycleTimer.start();
		loopIterations = 0;
	}

	/**
	 * @brief Get the number of task callbacks made so far
	 */
	unsigned getLoopIterations()
	{
		return loopIterations;
	}

	/**
	 * @brief Get the total number of CPU cycles since the last call to reset()
	 */
	uint32_t getElapsedCycles()
	{
		return cycleTimer.elapsedTicks();
	}

	/**
	 * @brief Get the figure used as the baseline cycle count
	 */
	uint32_t getMinLoopCycles()
	{
		return minLoopCycles;
	}

	/**
	 * @brief Get the CPU utilisation figure in 1/100ths of a percent
	 */
	unsigned getUtilisation()
	{
		auto elapsedCycles = getElapsedCycles();
		if(elapsedCycles <= minLoopCycles) {
			return 0;
		}

		auto maxIterations = elapsedCycles / minLoopCycles;
		if(loopIterations >= maxIterations) {
			return 0;
		}

		return 10000U * (maxIterations - loopIterations) / maxIterations;
	}

private:
	void loop()
	{
		++loopIterations;
		queueLoop();
	}

	void queueLoop()
	{
		System.queueCallback([](void* param) { static_cast<CpuUsage*>(param)->loop(); }, this);
	}

	void calibrationLoop()
	{
		++loopIterations;
		if(loopIterations < minCalIterations) {
			cycleTimer.start();
		} else if(loopIterations == minCalIterations + calIterations) {
			minLoopCycles = 8 * cycleTimer.elapsedTicks() / (calIterations * 10);
			// Calibration completed
			onReady();
			reset();
			queueLoop();
			return;
		}
		queueCalibrationLoop();
	}

	void queueCalibrationLoop()
	{
		System.queueCallback([](void* param) { static_cast<CpuUsage*>(param)->calibrationLoop(); }, this);
	}

private:
	static constexpr uint32_t minCalIterations = 0x20000;
	static constexpr uint32_t calIterations = 0x40000;
	CpuCycleTimer cycleTimer;
	unsigned loopIterations = 0;
	uint32_t minLoopCycles = 0; // Set during calibration
	InterruptCallback onReady = nullptr;
};

} // namespace Profiling
