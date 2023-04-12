function init() {
  if (!"mediaDevices" in navigator ||
      !"getUserMedia" in navigator.mediaDevices) {
    handleError("Camera API is not available in your browser");
    return;
  }

  // get page elements
  const video = document.querySelector("#video");
  const buttons = document.querySelector("#buttons");
  const size = document.querySelector("#size");
  const snapshot = document.querySelector("#snapshot");
  const canvas = document.querySelector("#canvas");
  const nocamera = document.querySelector("#nocamera");
  const videoSelect = document.querySelector('select#videoSource');
  const play = document.querySelector("#play");
  const pause = document.querySelector("#pause");

  // camera status
  let playing = true;

  // current video stream
  let videoStream;

  // connect hover slots
  window.widget.onenter.connect(() => { 
    buttons.style.display = "flex";
    if (videoStream) {
      (playing ? pause : play).style.display = "block";
    }
  });
  
  window.widget.onleave.connect(() => { 
    buttons.style.display = "none"; 
    pause.style.display = "none";
    play.style.display = "none";
  });
  
  // connect event handler
  // reassign selected stream
  videoSelect.onchange = getStream;

  // resize widget to video aspect ratio
  video.onresize = () => {
      let width = window.innerWidth;
      let height = width * video.videoHeight / video.videoWidth;
      sankore.resize(width, height);
  };

  // play/pause
  play.onclick = () => {
    video.play();
    playing = true;
    play.style.display = "none";
    pause.style.display = "block";
  };
  
  pause.onclick = () => {
    video.pause();
    playing = false;
    pause.style.display = "none";
    play.style.display = "block";
  };
  
  // take snapshot
  snapshot.onclick = (event) => {
    event.cancelBubble = true;
    canvas.width = size.value;
    canvas.height = canvas.width * video.videoHeight / video.videoWidth;
    canvas.getContext("2d").drawImage(video, 0, 0, video.videoWidth, video.videoHeight, 0, 0, canvas.width, canvas.height);
    var data = canvas.toDataURL("image/jpeg");
    sankore.addObject(data, 0, 0, 0, 0, false);
  };

  // stop video stream
  function stopVideoStream() {
    if (videoStream) {
      videoStream.getTracks().forEach((track) => {
        track.stop();
      });
    }
  }
  
  // request list of media devices
  function getDevices() {
    return navigator.mediaDevices.enumerateDevices();
  }

  // extract video devices and build selection options
  function gotDevices(deviceInfos) {
    for (const deviceInfo of deviceInfos) {
      const option = document.createElement('option');
      option.value = deviceInfo.deviceId;
      if (deviceInfo.kind === 'videoinput') {
        option.text = deviceInfo.label || `Camera ${videoSelect.length + 1}`;
        videoSelect.appendChild(option);
      }
    }
    // show selection if more than one video device
    if (videoSelect.options.length <= 1) {
      videoSelect.style.display = "none";
    }
  }

  // request stream for selected device
  function getStream() {
    stopVideoStream();
    const videoSource = videoSelect.value;
    const constraints = {
      video: {
        deviceId: videoSource ? {exact: videoSource} : undefined,
        width: { ideal: 1920 }, 
        height: { ideal: 1080 }  
      }
    };
    return navigator.mediaDevices.getUserMedia(constraints).
      then(gotStream).catch(handleError);
  }

  // assign stream to video element
  function gotStream(stream) {
    videoStream = stream;
    videoSelect.selectedIndex = [...videoSelect.options].
      findIndex(option => option.text === stream.getVideoTracks()[0].label);
    video.srcObject = stream;
  }

  // display "no camera" symbol
  function handleError(error) {
    videoStream = undefined;
    video.style.display = "none";
    nocamera.style.display = "flex";
  }

  // initialize
  getStream().then(getDevices).then(gotDevices);
}
