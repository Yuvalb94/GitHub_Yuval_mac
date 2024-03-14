import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import glob
import datetime


def find_csv_filenames( path_to_dir, suffix=".csv" ):
    filenames = os.listdir(path_to_dir)
    csv_files = [filename for filename in filenames if filename.endswith( suffix )]
    return sorted(csv_files)

def get_date(filename):
    date = filename[:filename.find("__")]
    return date

def get_time(filename):
    time = filename[(filename.find("__")+2):-4]
    return time

def concat_data_in_folder(path_to_dir):
    files = sorted(glob.glob(path_to_dir + '/*.csv')) 
    scale_data = pd.read_csv(files[0])
    for file in files[1:]:
        temp = pd.read_csv(file)
        scale_data = pd.concat([scale_data, temp], ignore_index=True)
    return scale_data

def plot_data(data, xaxisby='hours from start', date_fmt = '%Y-%m-%d %H:%M:%S.%f', title='BirdScale readings over time', show=False):
    fig, ax = plt.subplots()

    # Extract the dates and times separately from the time column
    times = [datetime.datetime.strptime(time, f"{date_fmt}").strftime("%H:%M") for time in data["Time"]]
    dates = [datetime.datetime.strptime(time, f"{date_fmt}").strftime("%Y-%m-%d") for time in data["Time"]]

    # Calculate the ticks of the date shifts
    unique_dates = np.unique(dates)
    date_shift_ticks  = []
    for date in unique_dates:
        date_shift_ticks.append(dates.index(date))

    # Plot the weight data
    ax.plot(data.index, data['Weight'], marker='.', linestyle='-')

    # Edit time axis
    if xaxisby == 'datetime': 
        ax.set_xticks(data.index[::3600], labels=times[::3600])
        ax.set_xlabel("time(H_M)")
        
        # Plot vertical lines in positions of date shifts
        for date in date_shift_ticks:
            ax.axvline(x=date, color='red', linestyle='--')
    
            # Add the value of the index next to the vertical line
            ax.text(date, np.mean(ax.get_ylim()), str(dates[date]), color='red', rotation=90, va='center', ha='right')

    else: # Plot x-axis as hours from start
        ax.set_xticks(data.index[::3600], labels=np.arange(0, len(data.index[::3600])))
        ax.set_xlabel("hours from start")

    plt.xticks(rotation=90)                  
    ax.set_ylabel("weight(g)")
    plt.title(f"{title}")
    ax.legend()
    if show == True:
        plt.show()
    else:
        return fig, ax

def find_bird_on(data, on_thrd = 10, off_thrd = 2):
    # This function gets a dataframe with time and scale readings, and returns a list with the times and weights that were measures while the bird was on the scale

    bird_on_scale_events = [[], []]
    data_for_analysis = data
    on_times = []
    off_times = []
    while len(data_for_analysis['Weight'][data_for_analysis['Weight'] > 5]) > 0:
        
        # Calculate the index of the first 'on' indication and store the 'on' time
        on_index = data_for_analysis.index[data_for_analysis['Weight']>10][0] 
        on_times.append(data_for_analysis['Time'][on_index])

        # Cut the dataframe before the 'on' indication, and reset the index
        data_for_analysis = data_for_analysis[on_index:-1] 
        data_for_analysis = data_for_analysis.reset_index(drop=True)
        # print(data_for_analysis)
        # plot_data(data_for_analysis, xaxisby='datetime', show=True)

        # Calculate the index of the next 'off' indication and store the 'off' time
        off_index = data_for_analysis.index[data_for_analysis['Weight'] < 2][0]
        off_times.append(data_for_analysis['Time'][off_index])

        # Save the Weight and Time data of the current 'on' event
        bird_on_event = data_for_analysis[0:off_index]
        # if off_index > 10:
        bird_on_scale_events[0].append(bird_on_event['Weight'].to_list())
        bird_on_scale_events[1].append(bird_on_event['Time'].to_list())

        # Cut the current 'on' event from the dataframe and move on
        data_for_analysis = data_for_analysis[off_index:-1]
        data_for_analysis = data_for_analysis.reset_index(drop=True)
        # print(data_for_analysis)
        # plot_data(data_for_analysis, xaxisby='datetime', show=True)

    return bird_on_scale_events, on_times, off_times


if __name__ == "__main__":
    path_to_scale_data = r'/Users/cohenlab/Desktop/bird_scale_project/accumulated_data'
    files = sorted(glob.glob(path_to_scale_data + '/*.csv')) 
    print("\tnumber of files: ", len(files))
    filenames = find_csv_filenames(path_to_scale_data)
    
    strp_format = '%Y-%m-%d %H:%M:%S.%f' # This is the format for etracting datetime object from the 'Time' column string

    scale_data = concat_data_in_folder(path_to_scale_data)


    # scale_data_20_02 = pd.read_csv(f"{path_to_scale_data}/2024_02_20.csv")


    # print(scale_data)
    #set lower values fluctuations to zero
    # data = scale_data


    plot_data(scale_data, xaxisby='datetime', date_fmt = strp_format, show=True)








    # data_for_analysis['Weight'][data_for_analysis['Weight'] < 1] = 0

    # plot_data(scale_data, xaxisby='datetime', show=True)
    # plt.show()
    # print(scale_data.Time[0].strftime("%H_%M_%S"))
    # times = [datetime.datetime.strptime(time, '%Y-%m-%d %H:%M:%S.%f').strftime("%H:%M") for time in scale_data_13_02["Time"]]
    # dates = [datetime.datetime.strptime(time, '%Y-%m-%d %H:%M:%S.%f').strftime("%Y-%m-%d") for time in scale_data_13_02["Time"]]
    # date = datetime.datetime.strptime(scale_data_13_02['Time'][0], '%Y-%m-%d %H:%M:%S.%f').strftime("%Y-%m-%d")
    # print(dates[:10])
  
    # print(scale_data_13_02.index[date_ticks])
    # print([dates[tick] for tick in date_ticks])
    # plot_data(scale_data_20_02, xaxisby='datetime', date_fmt = strp_format, show=True)
    # fig, ax = plt.subplots()
    # ax.plot(scale_data_13_02.index, data['Weight'], marker='.', linestyle='-')

    # ax.set_xticks(data.index[::3600], labels=data.Time[::3600])
    # ax.set_xlabel("time(H_M_S)")

    # plt.xticks(rotation=90)                  
    # ax.set_ylabel("weight")
    
    # plt.show()





    # bird_on_scale = data_for_analysis[data_for_analysis['Weight'] > 5]
    # print(bird_on_scale['Weight'].value_counts().nlargest(10))

    # print("Mean Value = ", np.mean(bird_on_scale['Weight']))
    # bins = np.arange(17.5, 22.5, 0.01)

    # plt.hist(bird_on_scale['Weight'], bins)
    # plt.show()

    # plot_data(data_for_analysis, xaxisby='datetime', show=True)
    
    # bird_on_scale_events, on_times, off_times = find_bird_on(data_for_analysis)

    # print(len(bird_on_scale_events[1]))
    # print("mean weight = ", np.mean([np.mean(event) for event in bird_on_scale_events[0]]))


    


    ## manual figure making:

    # fig, ax = plt.subplots()

    # times = [datetime.datetime.strptime(time, "%H_%M_%S").strftime("%H:%M") for time in data["Time"]]
    # print(times[:10])
    # # dates = [datetime.datetime.strptime(time, f"{date_fmt}").strftime("%Y-%m-%d") for time in scale_data_13_02["Time"]]

    # date_shift_ticks  = [np.where(data['Time'] == '00_00_01')[0].tolist()]
    # print(date_shift_ticks[0])
    # # Plot the weight data
    # ax.plot(data.index, data['Weight'], marker='.', linestyle='-')

    # # Edit time axis
    # ax.set_xticks(data.index[::3600], labels=times[::3600])
    # ax.set_xlabel("time(H_M)")
    # ax.axvline(x=date_shift_ticks[0][0], color='red', linestyle='--', label='Day Shift')
    # for date in date_shift_ticks[0][1:]:
        
    #     ax.axvline(x=date, color='red', linestyle='--')

    # plt.xticks(rotation=90)                  
    # ax.set_ylabel("weight (g)")
    # plt.title("BirdScale weight readings over five days")
    # ax.legend()
    # plt.show()