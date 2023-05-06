async function init() {
  if (!"mediaDevices" in navigator ||
      !"getUserMedia" in navigator.mediaDevices) {
    handleError("Camera API is not available in your browser");
    return;
  }

  // get page elements
  const video = document.querySelector("#video");
  const buttons = document.querySelector("#buttons");
  const buttonsLeft = document.querySelector("#buttons-left");
  const size = document.querySelector("#size");
  const snapshot = document.querySelector("#snapshot");
  const rotleft = document.querySelector("#rotleft");
  const rotright = document.querySelector("#rotright");
  const canvas = document.querySelector("#canvas");
  const nocamera = document.querySelector("#nocamera");
  const videoSelect = document.querySelector('select#videoSource');
  const play = document.querySelector("#play");
  const pause = document.querySelector("#pause");

  // camera status
  let playing = true;

  // current video stream
  let videoStream;
  
  // orientation
  let orientation = await sankore.async.preference("orientation", "0");

  // connect hover slots
  window.widget.onenter.connect(() => { 
    buttons.style.display = "flex";
    buttonsLeft.style.display = "flex";
    if (videoStream) {
      (playing ? pause : play).style.display = "block";
    }
  });
  
  window.widget.onleave.connect(() => { 
    buttons.style.display = "none"; 
    buttonsLeft.style.display = "none";
    pause.style.display = "none";
    play.style.display = "none";
  });
  
  // connect event handler
  // reassign selected stream
  videoSelect.onchange = getStream;

  // resize widget to video aspect ratio
  video.onresize = () => {
      let size = layoutVideo();
      sankore.resize(size.width, size.height);
  };

  // adjust video size when widget is resized
  window.onresize = layoutVideo;

  // rotate
  rotleft.onclick = () => {
    orientation = (orientation + 3) % 4;
    sankore.setPreference("orientation", orientation);
    let size = layoutVideo();
    sankore.resize(size.width, size.height);
  };

  rotright.onclick = () => {
    orientation = (orientation + 1) % 4;
    sankore.setPreference("orientation", orientation);
    let size = layoutVideo();
    sankore.resize(size.width, size.height);
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

    if (orientation % 2) {
      canvas.height = size.value;
      canvas.width = canvas.height * video.videoHeight / video.videoWidth;
    } else {
      canvas.width = size.value;
      canvas.height = canvas.width * video.videoHeight / video.videoWidth;
    }

    let ctx = canvas.getContext("2d");
    ctx.setTransform(1, 0, 0, 1, 0, 0);
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.translate(canvas.width / 2, canvas.height / 2);
    ctx.rotate(orientation * Math.PI / 2);

    if (orientation % 2) {
      ctx.translate(-canvas.height / 2, -canvas.width / 2);
      ctx.drawImage(video, 0, 0, video.videoWidth, video.videoHeight, 0, 0, canvas.height, canvas.width);
    } else {
      ctx.translate(-canvas.width / 2, -canvas.height / 2);
      ctx.drawImage(video, 0, 0, video.videoWidth, video.videoHeight, 0, 0, canvas.width, canvas.height);
    }

    let data = canvas.toDataURL("image/jpeg");
    sankore.addObject(data, 0, 0, 0, 0, false);
  };

  // layout video
  function layoutVideo() {
    let longWindow = Math.max(window.innerHeight, window.innerWidth);
    let width;
    let height

    video.style.transform = "rotate(" + 90 * orientation + "deg)";

    if (orientation % 2 == 0) {
      // for landscape:
      // use longest side for width
      // adapt the height according to aspect ratio
      width = longWindow;
      height = width * video.videoHeight / video.videoWidth;

      video.style.width = width + "px";
      video.style.height = height + "px";
      video.style.top = "0px";
      video.style.left = "0px";
    } else {
      // for portrait:
      // use longest side for height
      // adapt width according to aspect ratio
      height = longWindow;
      width = height * video.videoHeight / video.videoWidth;

      video.style.width = height + "px";
      video.style.height = width + "px";
      let offset = (height - width) / 2;
      video.style.top = offset + "px";
      video.style.left = -offset + "px";
    }

    return {
      'width': width,
      'height': height
    };
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
