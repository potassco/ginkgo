#include <iostream>

#include <gtkmm/main.h>
#include <boost/program_options.hpp>

#include <ginkgo/utils/TextFile.h>
#include <ginkgo/FeedbackLoopEvaluation.h>

int main(int argc, char **argv)
{
	namespace po = boost::program_options;

	po::options_description description("Allowed options");
	description.add_options()
		("help,h", "display this help message")
		("input,i", po::value<std::string>(), "Input directory with benchmark results");

	po::variables_map variablesMap;
	po::store(po::parse_command_line(argc, argv, description), variablesMap);
	po::notify(variablesMap);

	if (variablesMap.count("help"))
	{
		std::cout << description;
		return EXIT_SUCCESS;
	}

	Gtk::Main application(argc, argv);
	ginkgo::feedbackLoop::evaluation::EvaluationWindow mainWindow;

	if (variablesMap.count("input") > 0)
		mainWindow.loadResultsFromDirectory(variablesMap["input"].as<std::string>());

	application.run(mainWindow);

	return EXIT_SUCCESS;
}
